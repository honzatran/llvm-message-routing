

#ifndef ROUTING_LOOPING_STATE_H
#define ROUTING_LOOPING_STATE_H

#include <atomic>
#include <spdlog/fmt/fmt.h>

#include "hooks.h"
#include <routing/logger.h>

namespace routing
{

template <std::memory_order MEMORY_ORDER = std::memory_order_seq_cst>
class Looping_state : public Class_hook_guard<Looping_state<MEMORY_ORDER>>
{
public:
    Looping_state()
        : m_timeout(std::chrono::milliseconds{1})
    {
        m_logger = routing::get_default_logger("Looping_state");
        m_logger->debug("Looping state {} constructed", m_name);
    }

    Looping_state(
        std::string const& name,
        int count                         = 1,
        std::chrono::microseconds timeout = std::chrono::milliseconds{1})
        : Class_hook_guard<Looping_state<MEMORY_ORDER>>(name),
          m_name(name),
          m_timeout(timeout)
    {
        m_running.store(false);
        m_count.store(count);

        m_logger = routing::get_default_logger("Looping_state");

        m_logger->debug("Looping state {} constructed", m_name);
    }

    Looping_state(Looping_state const& other) = delete;
    Looping_state& operator=(Looping_state const& other) = delete;

    Looping_state(Looping_state && other) 
        : Class_hook_guard<Looping_state<MEMORY_ORDER>>(std::move(other)),
          m_name(std::move(other.m_name)),
          m_logger(std::move(other.m_logger))
    {
        m_running.store(other.m_running);
    }

    Looping_state& operator=(Looping_state && other)
    {
        if (this != &other)
        {
            Class_hook_guard<Looping_state<MEMORY_ORDER>>::operator=(
                std::move(other));

            m_name = std::move(m_name);
            m_running.store(other.m_running);
            m_logger = std::move(other.m_logger);
        }

        return *this;
    }

    ~Looping_state()
    {
        m_running.store(false);
    }

    void on_loop_finished()
    {
        m_count--;
    }

    void stop()
    {
        m_logger->debug("Stopping looping state {}", m_name);
        m_running.store(false, MEMORY_ORDER);
    }

    void start()
    {
        m_running.store(true, MEMORY_ORDER);
    }

    bool running() const
    {
        return m_running.load(MEMORY_ORDER);
    }

    void execute_hook_impl()
    {
        stop_and_wait();
    }

private:
    Logger_t m_logger = nullptr;

    std::string m_name = "default";
    std::atomic_bool m_running;

    std::atomic_int m_count;
    std::chrono::microseconds m_timeout;

    void stop_and_wait()
    {
        stop();

        if (!wait_on_loop_finished())
        {
            m_logger->warn("Loops not finished");
        }
    }

    bool wait_on_loop_finished()
    {
        auto start = std::chrono::system_clock::now();

        do
        {
            // here we are just spinning to avoid syscalls inside handlers 
        } while (m_count.load() > 0
                 && std::chrono::system_clock::now() - start < m_timeout);

        return (m_count.load() > 0);
    }
};

using Looping_state_relaxed = Looping_state<std::memory_order_relaxed>;
using Looping_state_t = Looping_state<std::memory_order_seq_cst>;

}  // namespace routing

#endif
