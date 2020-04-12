

#include "grpc_options.h"

#include <cstdint>
#include <string>
#include "spdlog/fmt/bundled/core.h"

std::string
get_channel_name(std::string_view name)
{
    return fmt::format("grpc.{}.channel", name);
}

program_options::options_description
routing::grpc::get_grpc_options_description(
    std::string_view name,
    std::uint16_t port)
{
    program_options::options_description grpc_options("grpc");

    auto channel_name    = get_channel_name(name);
    auto channel_default = fmt::format("0.0.0.0:{}", port);

    grpc_options.add_options()(
        channel_name.c_str(),
        routing::create_option<std::string>(channel_default),
        "channel where the engine listens to");

    return grpc_options;
}

routing::grpc::Grpc_option
routing::grpc::get_grpc_options(
    std::string_view name,
    routing::Config const& config)
{
    auto channel_name = get_channel_name(name);
    return {config.get_option<std::string>(get_channel_name(name))};
}
