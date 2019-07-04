

#ifndef ROUTING_THROTTLE_H
#define ROUTING_THROTTLE_H

#include <routing/fmt.h>

#include <boost/circular_buffer.hpp>
#include <chrono>

namespace routing
{

template <typename CLOCK>
class Throttle 
{
public:
    Throttle() = default;

    Throttle(std::size_t count)
    {
        setup(count);
    }

    bool crossed()
    {
        return (is_full() && cross_rate());
    }

    bool is_full() const
    {
        return m_timestamps.full();
    }

    void setup(std::size_t count)
    {
        m_timestamps.set_capacity(count);
    }

    void update()
    {
        auto now = CLOCK::now();
        m_timestamps.push_back(now);
    }

    std::size_t capacity() const
    {
        return m_timestamps.capacity();
    }

private:
    boost::circular_buffer<typename CLOCK::time_point> m_timestamps;

    bool cross_rate() const
    {
        if (m_timestamps.empty())
        {
            return true;
        }

        auto now = CLOCK::now();

        auto diff = now - m_timestamps.front();

        return diff < std::chrono::seconds{1};
    }
};



};

#endif
