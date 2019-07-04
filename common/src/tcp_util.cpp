

#include <routing/tcp_util.h>
#include <strings.h>

#include <routing/fmt.h>

using namespace routing;

sockaddr_in
routing::to_sockaddr_in(std::string const& host, uint16_t port)
{
    sockaddr_in server;

    bzero(reinterpret_cast<void *>(&server), sizeof(server));

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(host.c_str());
    server.sin_port = htons(port);

    return server;
}

void
routing::add_socket_option(
    Config& config,
    std::string const& socket_name,
    std::string const& host,
    std::uint16_t port)
{
    std::string host_option_name = fmt::format("{}.host", socket_name);
    std::string port_option_name = fmt::format("{}.port", socket_name);

    config.put_option<std::string>(host_option_name, host);
    config.put_option<std::uint16_t>(port_option_name, port);
}
