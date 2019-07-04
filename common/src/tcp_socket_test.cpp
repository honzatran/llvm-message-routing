
#include <routing/tcp_socket.h>
#include <routing/config.h>
#include <routing/logger.h>
#include <routing/idle_strategy.h>
#include <routing/testing.h>
#include <routing/latches.h>

#include <gtest/gtest.h>

#include <vector>

using namespace routing;
using namespace std;

class Tcp_socket_test : public ::testing::Test
{
public:

    static void SetUpTestCase() 
    {
        m_port_delta.store(
            0,
            std::memory_order_seq_cst);
    }

    void SetUp() override 
    {
        auto config_options = 
        {
            get_logger_options_description(),
            get_initiator_socket_option("tcp_socket.initiator"),
            get_initiator_socket_option("tcp_socket.acceptor")
        };

        Config config = 
            routing::parse_args(config_options, TEST_CONFIG_PATH);

        m_logger = routing::get_default_logger("Tcp_socket_test");
        m_logger->info("Tcp socket test started");

        std::uint16_t port = 
            config.get_option<std::uint16_t>("tcp_socket.initiator.port");

        std::uint16_t port_delta = 
            m_port_delta.load(std::memory_order_seq_cst);

        config.put_option<std::uint16_t>(
                "tcp_socket.initiator.port", 
                port + port_delta);

        config.put_option<std::uint16_t>(
                "tcp_socket.acceptor.port", 
                port + port_delta);

        m_acceptor = make_unique<Tcp_acceptor_socket>(
                "tcp_socket.acceptor", 
                config);

        m_initiator = make_unique<Tcp_initiator_socket>(
                "tcp_socket.initiator",
                config);

        // connect_latch.reset(1);
    }

    void TearDown() override
    {
        m_acceptor = nullptr;
        m_initiator = nullptr;

        std::this_thread::sleep_for(chrono::seconds{2});

        m_port_delta.fetch_add(1, std::memory_order_seq_cst);
    }

protected:

    std::shared_ptr<spdlog::logger> m_logger;

    std::unique_ptr<Tcp_acceptor_socket> m_acceptor;
    std::unique_ptr<Tcp_initiator_socket> m_initiator;

    std::thread create_server_thread(
        Tcp_listener<Tcp_acceptor_socket> *acceptor_listener,
        Event_latch* connect_latch,
        std::function<std::size_t(Buffer_view)> const& data_listener);

    std::thread create_client_thread(
        Tcp_listener<Tcp_initiator_socket> *initiator_listener,
        std::function<std::size_t(Buffer_view)> const& data_listener);

    std::thread create_server_poller_thread(
        Tcp_pollable<Tcp_acceptor_socket> *acceptor_listener,
        std::atomic_bool const& running,
        Event_latch* connect_latch,
        std::function<std::size_t(Buffer_view)> const& data_listener);

    std::thread create_client_poller_thread(
        Tcp_pollable<Tcp_initiator_socket> *initiator_listener,
        std::atomic_bool const& running,
        std::function<std::size_t(Buffer_view)> const& data_listener);

private:

    static std::atomic<std::uint16_t> m_port_delta;
};

std::atomic<std::uint16_t> Tcp_socket_test::m_port_delta(0);

std::thread Tcp_socket_test::create_server_thread(
        Tcp_listener<Tcp_acceptor_socket> *acceptor_listener,
        Event_latch* connect_latch,
        std::function<std::size_t(Buffer_view)> const& data_listener)
{
    return std::thread([this, acceptor_listener, &data_listener, connect_latch] 
    {
        std::error_code acceptor_error;
        m_acceptor->start_listening(acceptor_error);

        EXPECT_FALSE(acceptor_error) << "Acceptor listen failed due to" 
            << acceptor_error.message()
            << endl;

        connect_latch->signal();

        m_acceptor->wait_for_incoming_connection(acceptor_error);

        EXPECT_FALSE(acceptor_error) 
            << "Acceptor wair for incoming connection failed due to" 
            << acceptor_error.message()
            << endl;

        m_acceptor->set_data_listener(data_listener);

        acceptor_listener->start(
                Sleeping_idle_strategy{chrono::milliseconds{1}});
    });
}

std::thread Tcp_socket_test::create_client_thread(
    Tcp_listener<Tcp_initiator_socket> *initiator_listener,
    std::function<std::size_t(Buffer_view)> const& data_listener)
{
    return std::thread(
    [this, initiator_listener, data_listener] 
    {
        m_initiator->set_data_listener(data_listener);

        initiator_listener->start(
            Sleeping_idle_strategy{chrono::milliseconds{1}});
    });
}

std::thread Tcp_socket_test::create_server_poller_thread(
        Tcp_pollable<Tcp_acceptor_socket> *acceptor_listener,
        std::atomic_bool const& running,
        Event_latch* connect_latch,
        std::function<std::size_t(Buffer_view)> const& data_listener)
{
    return std::thread(
    [this, acceptor_listener, &running, &data_listener, connect_latch] 
    {
        std::error_code acceptor_error;
        m_acceptor->start_listening(acceptor_error);

        EXPECT_FALSE(acceptor_error) << "Acceptor listen failed due to"
                                     << acceptor_error.message() << endl;

        connect_latch->signal();

        m_acceptor->wait_for_incoming_connection(acceptor_error);

        EXPECT_FALSE(acceptor_error)
            << "Acceptor wair for incoming connection failed due to"
            << acceptor_error.message() << endl;

        std::error_code ec;
        while (running.load(std::memory_order_relaxed))
        {
            if (acceptor_listener->poll_for_messages(data_listener, ec) < 0)
            {
                break;
            }

            EXPECT_ERROR_CODE(ec, "POLL server failed");
        }
    });
}

std::thread Tcp_socket_test::create_client_poller_thread(
    Tcp_pollable<Tcp_initiator_socket> *initiator_pollable,
    std::atomic_bool const& running,
    std::function<std::size_t(Buffer_view)> const& data_listener)
{
    return std::thread([initiator_pollable, &running, data_listener] 
    {
        std::error_code ec;
        while (running.load(std::memory_order_relaxed))
        {
            if (initiator_pollable->poll_for_messages(data_listener, ec) < 0)
            {
                break;
            }

            EXPECT_ERROR_CODE(ec, "POLL client failed");
        }
    });
}


TEST_F(Tcp_socket_test, ping_pong_test)
{
    std::size_t const k_ping_ping_count = 100;

    Event_latch latch; 
    
    Latch message_received_latch(k_ping_ping_count);

    auto acceptor_listener = create_listener(m_acceptor.get());
    auto acceptor_sender = create_sender(m_acceptor.get());

    std::function<std::size_t(Buffer_view)> server_listener = 
    [&acceptor_sender] (Buffer_view view) -> std::size_t
    {
        std::size_t count = view.get_length()/sizeof(std::size_t);

        std::error_code send_code;

        EXPECT_TRUE(acceptor_sender);

        for (std::size_t i = 0; i < count; i++)
        {
            std::error_code send_code;

            EXPECT_TRUE(acceptor_sender);

            acceptor_sender->send(
                    view.slice(i * sizeof(std::size_t), sizeof(std::size_t)), 
                    send_code);

            EXPECT_FALSE(send_code)
                << "send failed due to "
                << send_code.message();
        }

        return view.get_length();
    };

    std::thread server_thread = 
        create_server_thread(acceptor_listener.get(), &latch, server_listener);
    
    ASSERT_EQ(
            cv_status::no_timeout, 
            latch.wait_for(chrono::seconds{5}));

    std::error_code client_code;

    m_initiator->start(client_code);

    ASSERT_FALSE(client_code) 
        << "Initiator connect failed due to" 
        << client_code.message();

    auto initiator_sender = create_sender(m_initiator.get());
    auto initiator_listener = create_listener(m_initiator.get());

    std::function<std::size_t(Buffer_view)> client_listener = 
    [&message_received_latch] (Buffer_view buffer_view)
    {
        std::size_t received_messages = 
            buffer_view.get_length()/sizeof(std::size_t);

        for (std::size_t i = 0; i < received_messages; i++) 
        {
            message_received_latch.count_down();
        }

        return buffer_view.get_length();
    };

    std::thread client_receiver = create_client_thread(
            initiator_listener.get(),
            client_listener);

    std::this_thread::sleep_for(chrono::seconds{2});

    for (std::size_t i = 0; i < k_ping_ping_count; i++)
    {
        auto data = vector<std::size_t> { std::size_t(i) };

        Buffer_view view(&data[0], 1);

        std::error_code send_code;
        initiator_sender->send(view, send_code);

        ASSERT_FALSE(send_code) 
            << "send failed due to "
            << send_code.message();
    }

    EXPECT_EQ(cv_status::no_timeout,
            message_received_latch.wait_for(chrono::seconds{20}));

    initiator_listener->stop();
    client_receiver.join();

    acceptor_listener->stop();
    server_thread.join();
}

TEST_F(Tcp_socket_test, ping_pong_poll)
{
    std::size_t const k_ping_ping_count = 100;

    Event_latch latch; 
    
    Latch message_received_latch(k_ping_ping_count);

    auto acceptor_listener = Tcp_pollable<Tcp_acceptor_socket>(m_acceptor.get());
    auto acceptor_sender = create_sender(m_acceptor.get());

    std::function<std::size_t(Buffer_view)> server_listener = 
    [&acceptor_sender] (Buffer_view view) -> std::size_t
    {
        std::size_t count = view.get_length()/sizeof(std::size_t);

        std::error_code send_code;

        EXPECT_TRUE(acceptor_sender);

        for (std::size_t i = 0; i < count; i++)
        {
            std::error_code send_code;

            EXPECT_TRUE(acceptor_sender);

            acceptor_sender->send(
                    view.slice(i * sizeof(std::size_t), sizeof(std::size_t)), 
                    send_code);

            EXPECT_FALSE(send_code)
                << "send failed due to "
                << send_code.message();
        }

        return count * sizeof(std::size_t);
    };

    std::atomic_bool server_running(true);

    std::thread server_thread = create_server_poller_thread(
        &acceptor_listener, server_running, &latch, server_listener);

    ASSERT_EQ(
            cv_status::no_timeout, 
            latch.wait_for(chrono::seconds{5}));

    std::error_code client_code;

    m_initiator->start(client_code);

    ASSERT_FALSE(client_code) 
        << "Initiator connect failed due to" 
        << client_code.message();

    auto initiator_sender = create_sender(m_initiator.get());
    auto initiator_listener
        = Tcp_pollable<Tcp_initiator_socket>(m_initiator.get());

    std::function<std::size_t(Buffer_view)> client_listener = 
    [&message_received_latch] (Buffer_view buffer_view)
    {
        std::size_t received_messages = 
            buffer_view.get_length()/sizeof(std::size_t);

        for (std::size_t i = 0; i < received_messages; i++) 
        {
            message_received_latch.count_down();
        }

        return received_messages * sizeof(std::size_t);
    };

    std::atomic_bool running(true);

    std::thread client_receiver = create_client_poller_thread(
            &initiator_listener,
            running,
            client_listener);

    std::this_thread::sleep_for(chrono::seconds{2});

    for (std::size_t i = 0; i < k_ping_ping_count; i++)
    {
        auto data = vector<std::size_t> { std::size_t(i) };

        Buffer_view view(&data[0], 1);

        std::error_code send_code;
        initiator_sender->send(view, send_code);

        ASSERT_FALSE(send_code) 
            << "send failed due to "
            << send_code.message();
    }

    EXPECT_EQ(cv_status::no_timeout,
            message_received_latch.wait_for(chrono::seconds{20}));

    // initiator_listener->stop();
    running.store(false);
    client_receiver.join();

    server_running.store(false);
    server_thread.join();
}

TEST_F(Tcp_socket_test, rebuff_test)
{
    std::size_t k_message_count = 2;
    Event_latch latch;
    
    Latch message_received_latch(k_message_count);

    auto acceptor_listener = create_listener(m_acceptor.get());
    auto acceptor_sender = create_sender(m_acceptor.get());

    std::size_t counter = 0;
    Latch second_part_msg_latch(2);
   
    std::function<std::size_t(Buffer_view)> server_listener = 
    [counter, k_message_count,&second_part_msg_latch, &acceptor_sender] 
    (Buffer_view view) mutable
    {
        std::size_t start = 0;
        while (start < view.get_length()) 
        {
            if (counter < k_message_count)
            {
                second_part_msg_latch.count_down();
                counter++;
            }

            std::size_t length = view.as_value<std::size_t>(start);
            std::size_t byte_length = length * sizeof(std::size_t);

            if (byte_length > view.get_length() - start)
            {
                break;
            }

            std::error_code ec;

            acceptor_sender->send(
                    view.slice(start, byte_length + sizeof(std::size_t)),
                    ec);

            EXPECT_FALSE(ec) 
                << "Sending failure due to: "
                << ec.message();

            start += byte_length + sizeof(std::size_t);
        }

        return start;
    };

    std::thread server_thread = 
        create_server_thread(acceptor_listener.get(), &latch, server_listener);

    ASSERT_EQ(
            cv_status::no_timeout, 
            latch.wait_for(chrono::seconds{5}));

    std::error_code client_code;
    m_initiator->start(client_code);

    ASSERT_FALSE(client_code) 
        << "Initiator connect failed due to" 
        << client_code.message();

    auto initiator_sender = create_sender(m_initiator.get());
    auto initiator_listener = create_listener(m_initiator.get());

    std::function<std::size_t(Buffer_view)> client_listener = 
    [&message_received_latch] (Buffer_view view)
    {
        for (std::size_t i = 0; i < view.get_length()/808; i++)
        {
            message_received_latch.count_down();
        }
        return view.get_length();
    };

    std::thread client_receiver = create_client_thread(
            initiator_listener.get(),
            client_listener);

    std::this_thread::sleep_for(chrono::seconds{2});

    auto first_msg = vector<std::size_t>(101, 1);
    first_msg[0] = 100;

    auto second_msg_first_part = vector<std::size_t>(51, 2);
    auto second_msg_second_part = vector<std::size_t>(50, 2);
    second_msg_first_part[0] = 100;

    std::error_code send_code;
    initiator_sender->send(
            Buffer_view(&first_msg[0], first_msg.size()), 
            send_code);

    ASSERT_FALSE(send_code) 
        << "send failed due to "
        << send_code.message();

    initiator_sender->send(
            Buffer_view(&second_msg_first_part[0], second_msg_first_part.size()), 
            send_code);

    ASSERT_FALSE(send_code) 
        << "send failed due to "
        << send_code.message();

    EXPECT_EQ(cv_status::no_timeout, 
            second_part_msg_latch.wait_for(chrono::seconds{2}));

    initiator_sender->send(
            Buffer_view(
                &second_msg_second_part[0], second_msg_second_part.size()), 
            send_code);

    EXPECT_EQ(cv_status::no_timeout,
            message_received_latch.wait_for(chrono::seconds{2}));

    initiator_listener->stop();
    client_receiver.join();

    acceptor_listener->stop();
    server_thread.join();
}

TEST_F(Tcp_socket_test, accept_connection)
{
    Tcp_connection_acceptor<Tcp_acceptor_socket> connection_acceptor(
        m_acceptor.get());

    Event_latch latch;

    std::error_code ec;

    m_acceptor->start_listening(ec);

    EXPECT_ERROR_CODE(ec, "Could not bind the acceptor socket");

    ASSERT_TRUE(connection_acceptor.wait_for_incoming_connection(
        [](std::error_code ec) { FAIL(); }, [&latch] { latch.signal(); }));

    ASSERT_FALSE(connection_acceptor.wait_for_incoming_connection(
        [](std::error_code ec) { FAIL(); }, [&latch] { latch.signal(); }));

    auto start = chrono::steady_clock::now();

    do
    {
        m_initiator->start(ec);
        std::this_thread::sleep_for(chrono::milliseconds{50});

    } while (ec && chrono::steady_clock::now() - start < chrono::seconds{5});

    EXPECT_ERROR_CODE(ec, "connection to server not established");

    EXPECT_EQ(cv_status::no_timeout, latch.wait_for(chrono::seconds{2}));

    m_logger->info("Closing the socket");
    m_initiator->close();

    Event_latch latch2;

    ASSERT_TRUE(connection_acceptor.wait_for_incoming_connection(
        [](std::error_code ec) { FAIL(); }, [&latch2] { latch2.signal(); }));

    ASSERT_FALSE(connection_acceptor.wait_for_incoming_connection(
        [](std::error_code ec) { FAIL(); }, [&latch2] { latch2.signal(); }));

    do
    {
        m_initiator->start(ec);
        std::this_thread::sleep_for(chrono::milliseconds{50});

    } while (ec && chrono::steady_clock::now() - start < chrono::seconds{5});

    EXPECT_EQ(cv_status::no_timeout, latch2.wait_for(chrono::seconds{2}));
}

