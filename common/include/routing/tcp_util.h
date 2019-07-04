

#ifndef ROUTING_TCP_UTIL_H
#define ROUTING_TCP_UTIL_H


#include <routing/config.h>
#include <routing/poller.h>
#include <routing/tcp_socket.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>

#include <string>

namespace routing
{
void
add_socket_option(
    Config& config,
    std::string const& socket_name,
    std::string const& host,
    std::uint16_t port);

sockaddr_in to_sockaddr_in(std::string const& host, uint16_t port);

template <typename SOCKET>
class Count_receive_pollable
{
public:
    Count_receive_pollable(SOCKET* socket)
        : m_socket_pollable(socket)
    {
    }

    Poll_result poll()
    {
        auto recv = [this] (Buffer_view view)
        {
            m_received++;

            return view.get_length();
        };

        std::error_code ec;

        int polled = m_socket_pollable.poll_for_messages(recv, ec);

        if (polled < 0)
        {
            return Poll_result::stop();
        }

        return Poll_result(polled);
    }

private:
    Tcp_pollable<SOCKET> m_socket_pollable;
    std::size_t m_received;
};
}


#endif
