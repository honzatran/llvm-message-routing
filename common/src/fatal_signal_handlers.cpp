
#include <routing/configuration.h>
#include <routing/fatal_signal_handlers.h>
#include <routing/logger.h>
#include <routing/stacktrace.h>
#include <routing/stacktrace_printer.h>

#include <absl/debugging/failure_signal_handler.h>

#include <csignal>

using namespace routing;
using namespace std;

void
routing::install_fatal_signal_handler()
{
    absl::FailureSignalHandlerOptions options;

    options.call_previous_handler = true;

    absl::InstallFailureSignalHandler(options);
}
