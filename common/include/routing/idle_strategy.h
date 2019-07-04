

#ifndef ROUTING_IDLE_STRATEGY_H
#define ROUTING_IDLE_STRATEGY_H

#include <chrono>
#include <thread>


class Sleeping_idle_strategy
{
public:

    template <typename DURATION>
    Sleeping_idle_strategy(DURATION timeout)
    {
        m_timeout = std::chrono::duration_cast<std::chrono::microseconds>(
                timeout);
    }

    Sleeping_idle_strategy()
        : m_timeout(std::chrono::milliseconds{1})
    {
    }

    Sleeping_idle_strategy(Sleeping_idle_strategy const& other) = default;
    Sleeping_idle_strategy& operator=(Sleeping_idle_strategy const& other) 
        = default;

    Sleeping_idle_strategy(Sleeping_idle_strategy&& other) = default;
    Sleeping_idle_strategy& operator=(Sleeping_idle_strategy&& other) 
        = default;

    void operator() () const
    {
        std::this_thread::sleep_for(m_timeout);
    }

    void operator()(int work_count) const 
    {
        if (work_count == 0)
        {
            std::this_thread::sleep_for(m_timeout);
        }
    }

private:

    std::chrono::microseconds m_timeout;
};

class Noop_idle_strategy
{
public:
    void operator()() const {}

    void operator()(int work_count) const {}
};

#endif
