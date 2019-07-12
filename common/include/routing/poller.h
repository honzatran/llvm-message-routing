
#ifndef ROUTING_POLLER_H
#define ROUTING_POLLER_H

#include <routing/thread_factory.h>
#include <routing/fmt.h>
#include <routing/logger.h>
#include <routing/idle_strategy.h>
#include <routing/portability.h>
#include <routing/detect.h>

#include <boost/lockfree/policies.hpp>
#include <boost/lockfree/queue.hpp>

#include <thread>
#include <atomic>
#include <type_traits>
#include <mutex>


namespace routing
{

/// Poller result is returned from each pollable inside from poll method
class Poll_result
{
public:
    constexpr explicit Poll_result(int work_count) : m_work_count(work_count) {}

    bool should_stop() const
    {
        return m_work_count < 0;
    }

    int work_count() const
    {
        return m_work_count;
    }

    constexpr static Poll_result stop()
    {
        return Poll_result(-1);
    }

    constexpr static Poll_result no_work()
    {
        return Poll_result(0);
    }

    operator int() const
    {
        return m_work_count;
    }

private:
    int m_work_count;
};

namespace detail
{
class Poller_base
{
public:
    Poller_base(std::string const& name)
        : m_name(name)
    {
        m_running.store(false);
    }

    void start(IThread_factory* thread_factory);

    void stop();

    ~Poller_base() 
    {
        stop();
    }

protected:
    virtual std::function<void()> get_run_loop() = 0;

    std::atomic_bool m_running;
    std::thread m_poller_thread;
    std::mutex m_mutex;

    std::string m_name;
};

template <typename POLLABLE>
using Has_on_start_method_t = decltype(std::declval<POLLABLE &>().on_start());

template <typename POLLABLE>
using Has_on_stop_method_t = decltype(std::declval<POLLABLE &>().on_stop());

struct Contains_method
{
};

struct No_method
{
};

template <bool value>
struct Has_method_base; 

template <>
struct Has_method_base<true> 
{
    using Tag_t = Contains_method;
};

template <>
struct Has_method_base<false> 
{
    using Tag_t = No_method;
};

template <typename POLLABLE>
struct Has_on_stop_method
    : public Has_method_base<is_detected<Has_on_stop_method_t, POLLABLE>::value>
{
};

template <typename POLLABLE>
struct Has_on_start_method
    : public Has_method_base<is_detected<Has_on_start_method_t, POLLABLE>::value>
{
};

}

/// Poller polls from multiple pollable, pollables must be registered
/// with the insert method
template <typename POLLABLE, typename IDLE_STRATEGY = Noop_idle_strategy>
class Poller final : public detail::Poller_base
{
    static_assert(
        std::is_copy_constructible<POLLABLE>::value,
        "POLLABLE Must be copy constructible");

    static_assert(
        std::is_default_constructible<POLLABLE>::value,
        "POLLABLE Must be default constructible");

public:
    using Pollable_t = POLLABLE;

    using Idle_strategy_t = IDLE_STRATEGY;
    Poller() : detail::Poller_base("POLLER"), m_insert_queue(10)
    {
        m_logger = routing::get_default_logger("POLLER");
    }

    Poller(
        std::string const& name,
        IDLE_STRATEGY idle_strategy = IDLE_STRATEGY())
        : Poller(name, 8, 8, idle_strategy)
    {
    }

    Poller(
        std::string const& name,
        int pollable_capacity,
        int insert_queue_capacity,
        IDLE_STRATEGY idle_strategy = IDLE_STRATEGY())
        : detail::Poller_base(name),
          m_insert_queue(insert_queue_capacity),
          m_idle_strategy(idle_strategy)
    {
        m_logger = routing::get_default_logger(
            fmt::format("POLLER_{}", name));
        
        m_to_remove.reserve(pollable_capacity);
        m_pollables.reserve(pollable_capacity);
    }

    /// Register the pollable with the poller
    bool insert(POLLABLE const& pollable) 
    {
        return m_insert_queue.push(pollable);
    }

    /// Start the poller
    /// void start(IThread_factory* thread_factory);
    /// implementation in the Poller_base

    /// Stops the poller
    /// void stop();
    /// implementation in the Poller_base

    /// testing purposes
    IDLE_STRATEGY const& get_idle_strategy()
    {
        return m_idle_strategy;
    }

protected:

    std::function<void()> get_run_loop() override
    {
        return [this]
        {
            run_impl();
        };
    }

    void run_impl() 
    {
        m_logger->info("Loop starts");

        while (m_running.load(std::memory_order_relaxed))
        { 
            int work_count = 0;

            for (std::size_t i = 0; i < m_pollables.size(); ++i)
            {
                Poll_result poll_result = m_pollables[i].poll();

                if (poll_result.should_stop())
                {
                    add_remove(i);
                } 
                else 
                {
                    work_count += poll_result;
                }
            }

            if (m_to_remove.size() > 0)
            {
                remove_stopped();
            }

            if (!m_insert_queue.empty())
            {
                insert_new_pollables();
            }

            m_idle_strategy(work_count);
        }

        invoke_on_stop_on_existing_pollables();

        m_logger->info("Loop quits");
    }

private:
    std::vector<POLLABLE> m_pollables;
    std::vector<int> m_to_remove;

    boost::lockfree::queue<POLLABLE> m_insert_queue;
    Logger_t m_logger;

    IDLE_STRATEGY m_idle_strategy;

    RA_DONT_INLINE
    void insert_new_pollables()
    {
        int const poll_count = 8;
        POLLABLE pollable;

        for (int i = 0; i < poll_count; ++i)
        {
            if (m_insert_queue.pop(pollable))
            {
                m_logger->debug("NEW POLLABLE PUSHED");
                m_pollables.push_back(pollable);


                typename detail::Has_on_start_method<POLLABLE>::Tag_t tag;
                invoke_on_start(m_pollables.back(), tag);

                m_to_remove.reserve(m_pollables.size());
            }
            else 
            {
                break;
            }
        }
    }

    RA_DONT_INLINE
    void add_remove(int index)
    {
        m_to_remove.push_back(index);
    }

    RA_DONT_INLINE
    void remove_stopped()
    {
        // remove pollables in the reverse order, this way the indices 
        // are correct even after the removal
        for (auto it = m_to_remove.rbegin(); it != m_to_remove.rend(); ++it)
        {
            m_logger->debug("REMOVE {}", *it);

            typename detail::Has_on_stop_method<POLLABLE>::Tag_t tag;
            invoke_on_stop(m_pollables[*it], tag);

            m_pollables.erase(m_pollables.begin() + *it);
        }

        m_to_remove.clear();
    }

    void invoke_on_start(POLLABLE& pollable, detail::No_method) {}

    void invoke_on_start(POLLABLE& pollable, detail::Contains_method) 
    {
        pollable.on_start();
    }

    void invoke_on_stop(POLLABLE& pollable, detail::No_method) {}

    void invoke_on_stop(POLLABLE& pollable, detail::Contains_method) 
    {
        pollable.on_stop();
    }

    void invoke_on_stop_on_existing_pollables() 
    {
        typename detail::Has_on_stop_method<POLLABLE>::Tag_t tag;

        for (POLLABLE& pollable : m_pollables)
        {
            invoke_on_stop(pollable, tag);
        }
    }
};

}

#endif
