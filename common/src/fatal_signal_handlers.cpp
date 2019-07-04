
#include <routing/fatal_signal_handlers.h>
#include <routing/logger.h>
#include <routing/stacktrace.h>
#include <routing/stacktrace_printer.h>
#include <routing/configuration.h>

#include <signal.h>

using namespace routing;
using namespace std;

struct
{
    int number;
    char const* name;
    struct sigaction old_action;
} k_fatal_signals[] = {
    {SIGSEGV, "SIGSEGV", {}},
    {SIGILL, "SIGILL", {}},
    {SIGABRT, "SIGABRT", {}},
    {SIGBUS, "SIGBUS", {}},
    {SIGTERM, "SIGTERM", {}},
    {SIGQUIT, "SIGQUIT", {}},
    {0, nullptr, {}},
};

// We are potentially leaking memory here, but we don't care about
// the destruction order
Signal_safe_printer* g_signal_safe_printer = new Signal_safe_printer();

constexpr pthread_t k_invalid_thread_id = 0;
std::atomic<pthread_t> g_signal_thread(k_invalid_thread_id);
std::atomic<bool> g_in_recursive_signal_handler(false);

void
dump_time_info()
{
    time_t now = time(nullptr);
    g_signal_safe_printer->print("*** Aborted at ");
    g_signal_safe_printer->print("{0} (Unix time, try 'date -d @{0}')", now);
    g_signal_safe_printer->print(" ***\n");

    g_signal_safe_printer->flush();
}

char const*
sigill_reason(int si_code)
{
    switch (si_code)
    {
        case ILL_ILLOPC:
            return "illegal opcode";
        case ILL_ILLOPN:
            return "illegal operand";
        case ILL_ILLADR:
            return "illegal addressing mode";
        case ILL_ILLTRP:
            return "illegal trap";
        case ILL_PRVOPC:
            return "privileged opcode";
        case ILL_PRVREG:
            return "privileged register";
        case ILL_COPROC:
            return "coprocessor error";
        case ILL_BADSTK:
            return "internal stack error";

        default:
            return nullptr;
    }
}

char const*
sigfpe_reason(int si_code)
{
    switch (si_code)
    {
        case FPE_INTDIV:
            return "integer divide by zero";
        case FPE_INTOVF:
            return "integer overflow";
        case FPE_FLTDIV:
            return "floating-point divide by zero";
        case FPE_FLTOVF:
            return "floating-point overflow";
        case FPE_FLTUND:
            return "floating-point underflow";
        case FPE_FLTRES:
            return "floating-point inexact result";
        case FPE_FLTINV:
            return "floating-point invalid operation";
        case FPE_FLTSUB:
            return "subscript out of range";

        default:
            return nullptr;
    }
}

char const*
sigsegv_reason(int si_code)
{
    switch (si_code)
    {
        case SEGV_MAPERR:
            return "address not mapped to object";
        case SEGV_ACCERR:
            return "invalid permissions for mapped object";

        default:
            return nullptr;
    }
}

const char*
sigbus_reason(int si_code)
{
    switch (si_code)
    {
        case BUS_ADRALN:
            return "invalid address alignment";
        case BUS_ADRERR:
            return "nonexistent physical address";
        case BUS_OBJERR:
            return "object-specific hardware error";

            // MCEERR_AR and MCEERR_AO: in sigaction(2) but not in headers.

        default:
            return nullptr;
    }
}

const char*
sigtrap_reason(int si_code)
{
    switch (si_code)
    {
        case TRAP_BRKPT:
            return "process breakpoint";
        case TRAP_TRACE:
            return "process trace trap";

        // TRAP_BRANCH and TRAP_HWBKPT: in sigaction(2) but not in headers.
        default:
            return nullptr;
    }
}

const char*
sigchld_reason(int si_code)
{
    switch (si_code)
    {
        case CLD_EXITED:
            return "child has exited ";
        case CLD_KILLED:
            return "child was killed ";
        case CLD_DUMPED:
            return "child terminated abnormally";
        case CLD_TRAPPED:
            return "traced child has trapped";
        case CLD_STOPPED:
            return "child has stopped";
        case CLD_CONTINUED:
            return "stopped child has continued";

        default:
            return nullptr;
    }
}

const char*
sigio_reason(int si_code)
{
    switch (si_code)
    {
        case POLL_IN:
            return "data input available";
        case POLL_OUT:
            return "output buffers available";
        case POLL_MSG:
            return "input message available";
        case POLL_ERR:
            return "I/O error";
        case POLL_PRI:
            return "high priority input available";
        case POLL_HUP:
            return "device disconnected";

        default:
            return nullptr;
    }
}

const char*
signal_reason(int signum, int si_code)
{
    switch (signum)
    {
        case SIGILL:
            return sigill_reason(si_code);
        case SIGFPE:
            return sigfpe_reason(si_code);
        case SIGSEGV:
            return sigsegv_reason(si_code);
        case SIGBUS:
            return sigbus_reason(si_code);
        case SIGTRAP:
            return sigtrap_reason(si_code);
        case SIGCHLD:
            return sigchld_reason(si_code);
        case SIGIO:
            return sigio_reason(si_code);  // aka SIGPOLL

        default:
            return nullptr;
    }
}

void
dump_signal_info(int signum, siginfo_t* siginfo)
{
    char const* name = nullptr;
    for (auto p = k_fatal_signals; p->name != nullptr; ++p)
    {
        if (p->number == signum)
        {
            name = p->name;
            break;
        }
    }

    g_signal_safe_printer->print("*** Signal {} ", signum);

    if (name != nullptr)
    {
        g_signal_safe_printer->print("({})", name);
    }

    g_signal_safe_printer->print(
        " ({}) received by PID {} pthread TID {:#X} linux TID {}",
        reinterpret_cast<std::uint64_t>(siginfo->si_addr),
        getpid(),
        (std::uint64_t)pthread_self(),
        syscall(__NR_gettid));

    if (siginfo->si_code != SI_KERNEL)
    {
        g_signal_safe_printer->print(
            " maybe from PID {}, UID {}", siginfo->si_pid, siginfo->si_uid);
    }

    g_signal_safe_printer->print(" ***\n");

    char const* reason = signal_reason(signum, siginfo->si_code);

    if (reason != nullptr)
    {
        g_signal_safe_printer->print("*** Code: {} ***\n", reason);
    }


    g_signal_safe_printer->flush();
}

void
inner_signal_handler(int signum, siginfo_t* info, void* uctx)
{
    pthread_t my_id = pthread_self();

    pthread_t previous_signal_thread = k_invalid_thread_id;

    while (
        !g_signal_thread.compare_exchange_strong(previous_signal_thread, my_id))
    {
        if (pthread_equal(previous_signal_thread, my_id) != 0)
        {
            if (!g_in_recursive_signal_handler.exchange(true))
            {
                g_signal_safe_printer->print(
                    "Entered fatal signal handler recursively. We're in "
                    "trouble.\n");
            }

            return;
        }

        timespec ts;
        ts.tv_sec  = 0;
        ts.tv_nsec = 100L * 1000 * 1000;  // 100 ms

        nanosleep(&ts, nullptr);

        previous_signal_thread = k_invalid_thread_id;
    }

    // dump stack trace
    //
    dump_time_info();

    dump_signal_info(signum, info);

    // g_signal_safe_printer->print("*** Stacktrace:\n");
    // g_signal_safe_printer->print_stacktrace();
    // g_signal_safe_printer->print("***\n");

    g_signal_safe_printer->flush();
}

void
raise_previous_signal_handler(int signum)
{
    for (auto p = k_fatal_signals; p->name != nullptr; ++p)
    {
        if (p->number == signum)
        {
            sigaction(signum, &p->old_action, nullptr);
            raise(signum);

            return;
        }
    }

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sigaction(signum, &sa, nullptr);
    raise(signum);
}

void
signal_handler(int signum, siginfo_t* info, void* uctx)
{
    inner_signal_handler(signum, info, uctx);

    raise_previous_signal_handler(signum);
}

std::atomic_bool g_already_installed;

void
routing::install_fatal_signal_handler()
{
    Logger_t logger = routing::get_default_logger("Signal handlers");

    logger->info("Installing the signal handlers");

    if (g_already_installed.exchange(true))
    {
        // already done
        logger->warn("Fatal signals already installed");
        return;
    }

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));

    sigemptyset(&sa.sa_mask);

    sa.sa_flags |= SA_SIGINFO | SA_ONSTACK;

    sa.sa_sigaction = &signal_handler;

    decltype(sa.sa_sigaction) tmp = &signal_handler;

    for (auto p = k_fatal_signals; p->name != nullptr; ++p)
    {
        logger->info("Installing signal {}", p->name);

        if (sigaction(p->number, &sa, &p->old_action) < 0)
        {
            logger->error(
                "Error installing signal {} because {}",
                p->number,
                strerror(errno));
        }
    }
}
