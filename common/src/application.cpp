

#include <routing/application.h>
#include <routing/hooks.h>

using namespace routing;

void
interrupt_handler(int signal)
{
    execute_all_hooks();

    std::exit(signal);
}

void
routing::init_interrupt_handler()
{
    signal(SIGINT, interrupt_handler);
}
