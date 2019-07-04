

#include <routing/poller_providers.h>
#include <routing/config.h>

#include <string>

using namespace routing;
using namespace std;

program_options::options_description
routing::get_poller_provider_options(std::string const& name)
{
    std::string poller_name
        = fmt::format("poller.provider.{}", name);

    auto socket_opt_description
        = program_options::options_description(poller_name);

    string poller_count    = fmt::format("{}.count", poller_name);
    string poller_capacity = fmt::format("{}.poller_capacity", poller_name);
    string poller_insert_capacity
        = fmt::format("{}.poller_capacity", poller_name);

    socket_opt_description.add_options()
        (
             poller_count.c_str(),
             create_option<std::size_t>(1),
             "number of pollers in the provider")
        (
             poller_capacity.c_str(),
             create_option<std::size_t>(8),
             "expected number of pollables within a single poller")
        (
            poller_insert_capacity.c_str(),
            create_option<std::size_t>(8),
            "expected capacity of the insert queue of the poller");


    return socket_opt_description;
}

Poller_provider_options
routing::extract_poller_options(Config const& config, std::string const& name)
{
    std::string poller_name = fmt::format("poller.provider.{}", name);

    string poller_count    = fmt::format("{}.count", poller_name);
    string poller_capacity = fmt::format("{}.poller_capacity", poller_name);
    string poller_insert_capacity
        = fmt::format("{}.poller_capacity", poller_name);

    return {config.get_option<std::size_t>(poller_count),
            config.get_option<std::size_t>(poller_capacity),
            config.get_option<std::size_t>(poller_insert_capacity)};
}
