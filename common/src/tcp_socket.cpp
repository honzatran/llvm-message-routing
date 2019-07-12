
#include <routing/tcp_socket.h>
#include <routing/logger.h>
#include <routing/tcp_util.h>

#include <type_traits>

#include <spdlog/fmt/fmt.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <errno.h>
#include <unistd.h>

namespace ps = program_options;

using namespace std;
using namespace routing;


template <typename F, typename ...ARGS>
int call_posix_method(std::error_code& ec, F posix_call, ARGS&&... args)
{
    int result = posix_call(std::forward<ARGS>(args)...);

    if (result < 0)
    {
        ec.assign(errno, std::system_category());
    }

    return result;
}

ps::options_description
routing::get_initiator_socket_option(std::string const& socket_name)
{
    auto socket_opt_description = ps::options_description(socket_name);

    string host_option = fmt::format("{}.host", socket_name);
    string port_option = fmt::format("{}.port", socket_name);

    socket_opt_description.add_options()
        (
             host_option.c_str(), 
             create_option<string>("127.0.0.1"),
             "initiator socket host")
        (
             port_option.c_str(), 
             create_option<uint16_t>(),
             "initiator socket port");

    return socket_opt_description;
}

ps::options_description
routing::get_acceptor_socket_option(std::string const& socket_name)
{
    auto socket_opt_description = ps::options_description(socket_name);

    string host_option = fmt::format("{}.host", socket_name);
    string port_option = fmt::format("{}.port", socket_name);

    socket_opt_description.add_options()
        (
             host_option.c_str(), 
             create_option<string>("127.0.0.1"),
             "acceptor socket host")
        (
             port_option.c_str(), 
             create_option<uint16_t>(),
             "acceptor socket port");

    return socket_opt_description;
}

pair<string, uint16_t> 
get_host(std::string const &socket_name, Config const& config) 
{
    string host_option = socket_name + ".host";
    string port_option = socket_name + ".port";

    return std::make_pair
        (
         config.get_option<string>(host_option),
         config.get_option<uint16_t>(port_option)
        );
}

detail::Tcp_socket_base::Tcp_socket_base(
        std::string const& name, 
        Config const& config)
    : m_name(name) 
{
    m_logger = routing::get_default_logger(name);

    std::tie(m_host, m_port) = get_host(m_name, config);
}

Tcp_initiator_socket::Tcp_initiator_socket(
    std::string const& name,
    Config const& config)
    : detail::Tcp_socket_base(name, config),
      hook_base_t(fmt::format("initiator.hook.{}", name)),
      m_connected(false)
{
}

Tcp_initiator_socket::~Tcp_initiator_socket()
{
    close();
}

void Tcp_initiator_socket::close()
{
    if (m_connected)
    {
        m_logger->info(
            "Closing initiator socket connected to {}:{}", m_host, m_port);
        ::close(m_socket_fd);

        m_connected = true;
    }
}

void
Tcp_initiator_socket::start(std::error_code &error_code)
{
    m_logger->info(
            "Initiator socket {} created connecting to host {} and port {} ", 
            m_name, 
            m_host, 
            m_port);

    int sock_fd
        = call_posix_method(error_code, ::socket, AF_INET, SOCK_STREAM, 0);

    sockaddr_in server = to_sockaddr_in(m_host, m_port);

    call_posix_method(
        error_code,
        ::connect,
        sock_fd,
        reinterpret_cast<sockaddr*>(&server),
        sizeof(server));


#if linux
    int one = 1;

    call_posix_method(
        error_code,
        ::setsockopt,
        sock_fd,
        SOL_TCP,
        TCP_NODELAY,
        &one,
        sizeof(one));
#endif

    if (error_code)
    {
        return;
    }

    m_connected = true;
    m_socket_fd = sock_fd;

    m_logger->debug("Initiator socket {}", m_socket_fd);

    m_logger->info("Initiator socket {} is connected", m_name);
}

Tcp_acceptor_socket::Tcp_acceptor_socket(
    std::string const& name,
    Config const& config)
    : detail::Tcp_socket_base(name, config),
      hook_base_t(fmt::format("acceptor.hook.{}", name)),
      m_listening(false),
      m_connected_to_the_client(false)
{
    // m_hook = Hook_guard(
    //     fmt::format("acceptor.hook.{}", m_name), [this] { close(); });
}

Tcp_acceptor_socket::~Tcp_acceptor_socket()
{
    close();
}

void Tcp_acceptor_socket::close()
{
    m_logger->info(
        "Closing acceptor socket listening on {}:{}", m_host, m_port);

    if (m_connected_to_the_client)
    {
        m_logger->info("Closing socket");

        if (::close(m_socket_fd) < 0)
        {
            m_logger->error("Closing failed because {}", strerror(errno));
        }

        m_connected_to_the_client = false;
    }

    if (m_listening)
    {
        m_logger->info("Closing listening socket");
        if (::close(m_listening_socket_fd) < 0)
        {
            m_logger->error("Closing failed because {}", strerror(errno));
        }

        m_listening = false;
    }
}

void
Tcp_acceptor_socket::start_listening(
    std::error_code& ec)
{
    if (m_listening)
    {
        return;
    }

    m_logger->info(
            "Acceptor socket {} created listening on host {} and port {} ", 
            m_name, 
            m_host, 
            m_port);

    // int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    int sock_fd = call_posix_method(ec, ::socket, AF_INET, SOCK_STREAM, 0);

    if (ec)
    {
        return;
    }

    int one = 1;
    call_posix_method(
        ec, ::setsockopt, sock_fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    
    if (ec)
    {
        return;
    }

    sockaddr_in server = to_sockaddr_in(m_host, m_port);

    call_posix_method(
            ec,
            ::bind,
            sock_fd,
            reinterpret_cast<sockaddr*>(&server),
            (socklen_t) sizeof(sockaddr));

    if (ec) 
    {
        return;
    }
    
    m_logger->info("Acceptor socket {} binded", m_name);

    call_posix_method(ec, ::listen, sock_fd, 1);

    if (ec)
    {
        return;
    }

    m_logger->info("Acceptor socket {} listening", m_name);

    m_listening_socket_fd = sock_fd;
    m_listening = true;
}

bool
Tcp_acceptor_socket::wait_for_incoming_connection(
    std::error_code& ec,
    std::chrono::milliseconds timeout)
{
    sockaddr_in client;
    int client_size = sizeof(sockaddr_in);

    fd_set rfds;

    FD_ZERO(&rfds);
    FD_SET(m_listening_socket_fd, &rfds);

    timeval tv;

    auto timeout_seconds = chrono::duration_cast<chrono::seconds>(timeout);
    auto timeout_microseconds = chrono::duration_cast<chrono::microseconds>(
        timeout - timeout_seconds);

    tv.tv_sec = timeout_seconds.count();
    tv.tv_usec = timeout_microseconds.count();

    int select_return = call_posix_method(
        ec, ::select, m_listening_socket_fd + 1, &rfds, nullptr, nullptr, &tv);

    if (ec)
    {
        return false;
    }

    if (select_return == 0)
    {
        // m_logger->info(
        //     "No incoming connection within {} ms", timeout.count());
        return false;
    }

    int client_socket = call_posix_method(
            ec,
            ::accept,
            m_listening_socket_fd,
            reinterpret_cast<sockaddr*>(&client),
            reinterpret_cast<socklen_t*>(&client_size));


#if linux
    int one = 1;
    call_posix_method(
        ec, ::setsockopt, client_socket, SOL_TCP, TCP_NODELAY, &one,
        sizeof(one));
#endif

    if (ec)
    {
        return false;
    }

    m_socket_fd = client_socket;

    m_logger->info("Acceptor socket {} connected", m_name);
    m_connected_to_the_client = true;

    return true;
}


void
detail::Kernel_socket_sender::send(Buffer_view view, std::error_code &ec) const
{
    std::size_t send = 0;

    while (send < view.get_length())
    {
        Buffer_view remaining = view.slice(send, view.get_length() - send);

        int send_result = ::send(
            m_socket_ptr->get_socket_fd(), remaining.as<void>(),
            remaining.get_length(),
            MSG_DONTWAIT);

        if (send_result < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                continue;
            }

            ec.assign(errno, std::system_category());
            break;
        }

        send += send_result;
    }
}

