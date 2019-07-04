
#ifndef ROUTING_DEBUG_H
#define ROUTING_DEBUG_H

#include <chrono>
#include <functional>

namespace routing
{

class Once_in_time
{
public:
    Once_in_time() = default;

    Once_in_time(
        std::chrono::microseconds period) : m_period(period)
    {
        m_invoked = std::chrono::system_clock::now();
    }

    template <typename FUNCTOR, typename... ARGS>
    void operator() (FUNCTOR functor, ARGS&&... args)
    {
        auto now = std::chrono::system_clock::now();

        if (now - m_invoked > m_period)
        {
            functor(std::forward<ARGS>(args)...);

            m_invoked = now;
        }
    }

    template <typename FUNCTOR, typename... ARGS>
    bool try_invoke(FUNCTOR functor, ARGS&&... args)
    {
        auto now = std::chrono::system_clock::now();

        if (now - m_invoked > m_period)
        {
            functor(std::forward<ARGS>(args)...);

            m_invoked = now;

            return true;
        }

        return false;
    }

private:
    std::chrono::microseconds m_period;
    std::chrono::system_clock::time_point m_invoked;
};

}

#endif
