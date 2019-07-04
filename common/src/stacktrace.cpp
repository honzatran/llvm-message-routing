

#include <routing/stacktrace.h>
#include <routing/stdext.h>
// #include "configuration.h"

#include <spdlog/fmt/fmt.h>
#include <cxxabi.h>
#include <unistd.h>
#include <stdio.h>

#if LIBUNWIND_ENABLED
#include <libunwind.h>
#endif


using namespace routing;

void
Symbol_frame::setup(std::uintptr_t offset, char const* symbol)
{
    m_offset = offset;
    m_symbol.assign(symbol);
    m_symbol_found = true;

    fmt::MemoryWriter writer;
}

int
stacktrace_impl(
    std::uintptr_t* addresses,
    Symbol_frame* symbol_frame,
    std::size_t addresses_count,
    bool demangle)
{
#if LIBUNWIND_ENABLED
    unw_context_t context;
    unw_cursor_t cursor;
    
    unw_getcontext(&context);
    unw_init_local(&cursor, &context);

    std::size_t i = 0;

    while (i < addresses_count && unw_step(&cursor))
    {
        unw_word_t offset, pc;

        unw_get_reg(&cursor, UNW_REG_IP, &pc);

        if (pc == 0)
        {
            break;
        }

        addresses[i] = pc;

        char sym[256];
        if (unw_get_proc_name(&cursor, sym, sizeof(sym), &offset) == 0)
        {
            char* nameptr = sym;
            int status;

            if (demangle)
            {
                char* demangled_name
                    = abi::__cxa_demangle(sym, nullptr, nullptr, &status);

                if (status == 0)
                {
                    nameptr = demangled_name;
                }

                symbol_frame[i].setup(offset, nameptr);

                std::free(demangled_name);
            }
            else 
            {
                symbol_frame[i].setup(offset, nameptr);
            }
        }

        i++;
    }

    return i;

#else
    return 0;
#endif
}


int routing::stacktrace(
    std::uintptr_t* addresses,
    Symbol_frame* symbol_frame,
    std::size_t addresses_count)
{
    return stacktrace_impl(addresses, symbol_frame, addresses_count, true);
}

int routing::signal_safe_stacktrace(
    std::uintptr_t* addresses,
    Symbol_frame* symbol_frame,
    std::size_t addresses_count)
{
    return stacktrace_impl(addresses, symbol_frame, addresses_count, false);
}

