
#include <routing/portability.h>
#include <routing/testing.h>
#include <routing/config.h>
#include <routing/logger.h>
#include <routing/fatal_signal_handlers.h>

#include <benchmark/benchmark.h>

#include <iostream>
#include <vector>

#include <gtest/gtest.h>

using namespace std;
using namespace routing;

ROUTING_ATTR_WEAK int main(int argc, char** argv);

vector<program_options::options_description>
get_all_subcomponent_desc()
{
    return {get_logger_options_description()};
}

int
main(int argc, char** argv)
{
    ::benchmark::Initialize(&argc, argv);

    routing::Config config = routing::parse_args(
        get_all_subcomponent_desc(), argc, argv, TEST_CONFIG_PATH);

    routing::init_logger(config);
    routing::install_fatal_signal_handler();

    if (::benchmark::ReportUnrecognizedArguments(argc, argv))
    {
        return 1;
    }

    return ::benchmark::RunSpecifiedBenchmarks();                            
}
