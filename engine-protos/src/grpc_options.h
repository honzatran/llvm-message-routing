
#include <routing/config.h>
#include <cstdint>

namespace routing::grpc
{
program_options::options_description
get_grpc_options_description(std::string_view name, std::uint16_t port);

struct Grpc_option
{
    std::string channel;
};

Grpc_option
get_grpc_options(std::string_view name, routing::Config const& config);
}  // namespace routing::grpc
