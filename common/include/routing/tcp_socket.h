
#ifndef ROUTING_TCP_SOCKET_H
#define ROUTING_TCP_SOCKET_H

#include <routing/config.h>
#include <routing/stdext.h>
#include "buffer.h"

#include "hooks.h"
#include <routing/logger.h>

#include <spdlog/spdlog.h>

#include <sys/socket.h>

#include <string>
#include <memory>
#include <iostream>
#include <atomic>

#include <type_traits>

namespace routing
{

program_options::options_description
get_initiator_socket_option(std::string const& socket_name);

program_options::options_description
get_acceptor_socket_option(std::string const& socket_name);

using Data_listener = 
    routing::Function_ref<std::size_t(routing::Buffer_view)>;

class ITcp_connector
{
public:
    virtual ~ITcp_connector() = default;

    virtual void set_data_listener(Data_listener data_listener) = 0;
};

class Tcp_acceptor_socket;
class Tcp_initiator_socket;

template <typename SOCKET>
class Tcp_pollable;

namespace detail
{

class Tcp_socket_base : public ITcp_connector
{
public:
    Tcp_socket_base(std::string const& name, Config const& config);

    Tcp_socket_base(Tcp_socket_base const& other) = delete;
    Tcp_socket_base& operator=(Tcp_socket_base const& other) = delete;

    Tcp_socket_base(Tcp_socket_base && other) = default;
    Tcp_socket_base& operator=(Tcp_socket_base && other) = default;

    void set_data_listener(Data_listener data_listener) override
    {
        m_data_listener = data_listener;
    }

    int get_socket_fd() const
    {
        return m_socket_fd;
    }

    Data_listener const get_data_listener() const
    {
        return m_data_listener;
    }

    std::string const& get_name() const
    {
        return m_name;
    }

protected:
    std::shared_ptr<spdlog::logger> m_logger;

    std::string m_name;

    std::string m_host;
    std::uint16_t m_port;

    Data_listener m_data_listener;
    int m_socket_fd;
};

template <typename SOCKET>
class Kernel_socket_listener_base
{
    static_assert(
        std::is_base_of<Tcp_socket_base, SOCKET>::value,
        "SOCKET must inherit from Tcp_socket_base");
public:
    Kernel_socket_listener_base(Tcp_socket_base * socket_ptr)
        : m_socket_ptr(socket_ptr), m_running(false)
    {
        m_logger = get_default_logger(
            fmt::format("tcp_listener.{}", m_socket_ptr->get_name()));
    }

protected:
    SOCKET const& get_socket_instance() const
    {
        return *reinterpret_cast<SOCKET const*>(m_socket_ptr);
    }

    SOCKET& get_socket_instance()
    {
        return *reinterpret_cast<SOCKET*>(m_socket_ptr);
    }

    template <typename IDLE_STRATEGY>
    void start_receiving_data(
        int socket_fd,
        Data_listener data_listener,
        IDLE_STRATEGY idle_strategy);

    void stopListenerLoop()
    {
        m_running.store(false, std::memory_order_relaxed);
    }

private:
    Tcp_socket_base *const m_socket_ptr;
    std::atomic_bool m_running;

    Logger_t m_logger;
};

template <typename SOCKET>
class Kernel_socket_pollable_base
{
    static_assert(
        std::is_base_of<Tcp_socket_base, SOCKET>::value,
        "SOCKET must inherit from Tcp_socket_base");

public:
    Kernel_socket_pollable_base() = default;

    Kernel_socket_pollable_base(
        Tcp_socket_base* socket_ptr,
        std::size_t buffer_size)
        : m_socket_ptr(socket_ptr), m_buffer(buffer_size)
    {
        m_logger = get_default_logger(
            fmt::format("tcp_poller.{}", socket_ptr->get_name()));
    }

    SOCKET const& get_socket_instance() const
    {
        return *reinterpret_cast<SOCKET const*>(m_socket_ptr);
    }

    SOCKET& get_socket_instance()
    {
        return *reinterpret_cast<SOCKET*>(m_socket_ptr);
    }

    /// returns < 0, if the connection was broken, otherwise number
    /// of read bytes from the socket
    template <typename FUNCTOR>
    int poll_for_messages(FUNCTOR&& functor, std::error_code& error_code)
    {
        std::size_t prev_position = m_buffer.get_position();

        int bytes_read = ::recv(
             m_socket_ptr->get_socket_fd(), 
             m_buffer.position_ptr_as<void>(),
             m_buffer.remaining(),
             MSG_DONTWAIT);

        if (bytes_read < 0) 
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                return 0;
            }
            else 
            {
                error_code.assign(errno, std::system_category());
                return -1;
            }
        }
        else  if (bytes_read == 0)
        {
            return -1;
        }

        Buffer_view buffer_view
            = m_buffer.slice_from_position(bytes_read);

        std::size_t processed = functor(buffer_view);

        m_buffer.reset(processed, prev_position + bytes_read - processed);

        return bytes_read;
    }

private:
    Tcp_socket_base* m_socket_ptr;

    Buffer m_buffer;
    Logger_t m_logger;
};


// implementation of receive loop
template <typename SOCKET>
template <typename IDLE_STRATEGY>
void 
Kernel_socket_listener_base<SOCKET>::start_receiving_data(
        int const socket_fd,
        Data_listener const data_listener,
        IDLE_STRATEGY const idle_strategy)
{
    Buffer buffer(1024 * 1024);

    m_running.store(true, std::memory_order_relaxed);

    while (m_running.load(std::memory_order_relaxed))
    {
        std::size_t prev_position = buffer.get_position();

        int bytes_read = ::recv(
             socket_fd, 
             buffer.position_ptr_as<void>(),
             buffer.remaining(),
             MSG_DONTWAIT);


        if (bytes_read < 0) 
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                idle_strategy();
                continue;
            }
            
            
                m_logger->info("listening stopped because {}", strerror(errno));
                break;
            
        }
        if (bytes_read == 0)
        {
            m_logger->info(
                "listening stopped because the other side closed connection");
            break;
        }

        Buffer_view buffer_view
            = buffer.slice_from_position(bytes_read);

        std::size_t processed = data_listener(buffer_view);

        buffer.reset(processed, prev_position + bytes_read - processed);
    }
}

class Kernel_socket_sender
{
public:
    Kernel_socket_sender() : m_socket_ptr(nullptr) {}

    Kernel_socket_sender(Tcp_socket_base const* socket_ptr)
        : m_socket_ptr(socket_ptr)
    {
        m_logger = get_default_logger(
            fmt::format("tcp_sender.{}", m_socket_ptr->get_name()));
    }

    void send(Buffer_view view, std::error_code &ec) const;

private:
    Tcp_socket_base const* m_socket_ptr;
    Logger_t m_logger;
};


};

template <typename ACCEPTOR_SOCKET> 
class Tcp_connection_acceptor;

template <typename SOCKET>
class Tcp_listener;


template <typename SOCKET>
class Tcp_sender;

class Tcp_initiator_socket : public detail::Tcp_socket_base,
                             public Class_hook_guard<Tcp_initiator_socket>
{
public:
    Tcp_initiator_socket(std::string const& name, Config const& config);
    ~Tcp_initiator_socket() override;

    Tcp_initiator_socket(Tcp_initiator_socket const& other) = delete;
    Tcp_initiator_socket& operator=(Tcp_initiator_socket const& other) = delete;

    Tcp_initiator_socket(Tcp_initiator_socket && other) = default;
    Tcp_initiator_socket& operator=(Tcp_initiator_socket && other) = default;

    void start(std::error_code &error_code);

    void close();

    void execute_hook_impl()
    {
        close();
    }

private:
    bool m_connected;
};

class Tcp_acceptor_socket : public detail::Tcp_socket_base,
                            public Class_hook_guard<Tcp_acceptor_socket>
{
public:
    Tcp_acceptor_socket(std::string const& name, Config const& config);
    ~Tcp_acceptor_socket() override;

    Tcp_acceptor_socket(Tcp_acceptor_socket const& other) = delete;
    Tcp_acceptor_socket& operator=(Tcp_acceptor_socket const& other) = delete;

    Tcp_acceptor_socket(Tcp_acceptor_socket && other) = default;
    Tcp_acceptor_socket& operator=(Tcp_acceptor_socket && other) = default;

    void start_listening(std::error_code& ec);

    bool wait_for_incoming_connection(
        std::error_code& ec,
        std::chrono::milliseconds timeout = std::chrono::seconds{5});

    void close();

    void execute_hook_impl()
    {
        close();
    }

private:
    int m_listening_socket_fd;
    bool m_listening;

    bool m_connected_to_the_client;

    void start_receiving_data();
};

template <>
class Tcp_connection_acceptor<Tcp_acceptor_socket>
{
public:
    Tcp_connection_acceptor(Tcp_acceptor_socket* socket) : m_socket(socket)
    {
        m_running.store(false, std::memory_order_relaxed);
        m_logger = routing::get_default_logger(
            fmt::format("Tcp_connection_acceptor.{}", m_socket->get_name()));
    }

    ~Tcp_connection_acceptor()
    {
        stop();
        if (m_accepting_thread.joinable())
        {
            m_accepting_thread.join();
        }
    }

    bool wait_for_incoming_connection(
        const std::function<void(std::error_code)>& on_error,
        const std::function<void()>& on_connection_accepted)
    {
        bool expected = false;
        
        if (!m_running.compare_exchange_strong(expected, true))
        {
            return false;
        }

        if (m_accepting_thread.joinable())
        {
            m_accepting_thread.join();
        }

        m_accepting_thread
            = std::thread([this, on_connection_accepted, on_error] {
                  m_logger->info("Waiting for incoming connection");

                  m_running.store(true, std::memory_order_seq_cst);
                  while (m_running.load(std::memory_order_seq_cst))
                  {
                      std::error_code ec;

                      if (m_socket->wait_for_incoming_connection(
                              ec, std::chrono::milliseconds{20}))
                      {
                          m_logger->info("Incoming connection accepted");
                          on_connection_accepted();
                          m_running.store(false, std::memory_order_seq_cst);
                          break;
                      }

                      if (ec)
                      {
                          on_error(ec);
                          m_running.store(false, std::memory_order_seq_cst);
                          break;
                      }
                  }

                  m_logger->info("Stop listening for incoming connection");
              });

        return true;
    }

    bool is_accepting_connection()
    {
        return m_running.load(std::memory_order_seq_cst);
    }

    void stop() 
    { 
        m_logger->info("Stop waiting for client connection invoked");
        m_running.store(false, std::memory_order_seq_cst); 
    }

private:
    Tcp_acceptor_socket* m_socket;

    std::atomic_bool m_running;
    std::thread m_accepting_thread;

    Logger_t m_logger;
};

template <>
class Tcp_listener<Tcp_initiator_socket>
    : protected detail::Kernel_socket_listener_base<Tcp_initiator_socket>
{
public:
    Tcp_listener() = default;

    Tcp_listener(Tcp_initiator_socket * socket_ptr)
        : detail::Kernel_socket_listener_base<Tcp_initiator_socket>(socket_ptr)
    {
    }

    template <typename IDLE_STRATEGY>
    void start(IDLE_STRATEGY idle_strategy)
    {
        auto const& socket = get_socket_instance();

        start_receiving_data(
                socket.get_socket_fd(), 
                socket.get_data_listener(),
                std::move(idle_strategy));
    }

    void stop() 
    {
        stopListenerLoop();
    }

    Tcp_initiator_socket& get_socket()  
    {
        return get_socket_instance();
    }

    Tcp_initiator_socket const& get_socket() const
    {
        return get_socket_instance();
    }
private:
};

template <>
class Tcp_listener<Tcp_acceptor_socket>
    : protected detail::Kernel_socket_listener_base<Tcp_acceptor_socket>
{
public:
    Tcp_listener(Tcp_acceptor_socket* socket_ptr)
        : detail::Kernel_socket_listener_base<Tcp_acceptor_socket>(socket_ptr)
    {
    }

    template <typename IDLE_STRATEGY>
    void start(IDLE_STRATEGY idle_strategy)
    {
        auto const& socket = get_socket_instance();

        start_receiving_data(
                socket.get_socket_fd(), 
                socket.get_data_listener(),
                std::move(idle_strategy));
    }

    void stop() 
    {
        stopListenerLoop();
    }

    Tcp_acceptor_socket& get_socket()  
    {
        return get_socket_instance();
    }

    Tcp_acceptor_socket const& get_socket() const
    {
        return get_socket_instance();
    }
private:
};

template <>
class Tcp_pollable<Tcp_initiator_socket>
    : public detail::Kernel_socket_pollable_base<Tcp_initiator_socket>
{
    using base_t = detail::Kernel_socket_pollable_base<Tcp_initiator_socket>;
public:
    Tcp_pollable(
        Tcp_initiator_socket* socket_ptr,
        std::size_t buffer_size = 8192)
        : base_t(socket_ptr, buffer_size)
    {
    }

    /// returns < 0, if the connection was broken, otherwise number
    /// of read bytes from the socket
    /// template <typename FUNCTOR>
    /// int poll_for_messages(FUNCTOR&& functor, std::error_code& error_code)
    /// implementation in Kernel socket poller base
    
private:
};


template <>
class Tcp_pollable<Tcp_acceptor_socket>
    : public detail::Kernel_socket_pollable_base<Tcp_acceptor_socket>
{
    using base_t = detail::Kernel_socket_pollable_base<Tcp_acceptor_socket>;
public:
    Tcp_pollable() = default;

    Tcp_pollable(
        Tcp_acceptor_socket* socket_ptr,
        std::size_t buffer_size = 8192)
        : base_t(socket_ptr, buffer_size)
    {
    }

    /// returns < 0, if the connection was broken, otherwise number
    /// of read bytes from the socket
    /// template <typename FUNCTOR>
    /// int poll_for_messages(FUNCTOR&& functor, std::error_code& error_code)
    /// implementation in Kernel socket poller base
    
private:
};

template<>
class Tcp_sender<Tcp_acceptor_socket>
    : public detail::Kernel_socket_sender
{
public:
    Tcp_sender() = default;

    Tcp_sender(Tcp_acceptor_socket const* socket)
        : detail::Kernel_socket_sender(socket)
    {
    }
};

template<>
class Tcp_sender<Tcp_initiator_socket>
    : public detail::Kernel_socket_sender
{
public:
    Tcp_sender() = default;

    Tcp_sender(Tcp_initiator_socket const* socket)
        : detail::Kernel_socket_sender(socket)
    {
    }
};



template <typename SOCKET_TYPE>
std::unique_ptr<Tcp_listener<SOCKET_TYPE>>
create_listener(SOCKET_TYPE *const socket)
{
    return make_unique<Tcp_listener<SOCKET_TYPE>>(socket);
}

template <typename SOCKET_TYPE>
std::unique_ptr<Tcp_sender<SOCKET_TYPE>>
create_sender(SOCKET_TYPE *const socket)
{
    return make_unique<Tcp_sender<SOCKET_TYPE>>(socket);
}

}  // namespace routing

#endif
