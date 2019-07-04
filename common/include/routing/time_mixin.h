

#ifndef ROUTING_TIME_MIXIN_H
#define ROUTING_TIME_MIXIN_H

#include <chrono>
#include <thread>

#include "time_util.h"
#include <routing/logger.h>

#include <spdlog/fmt/fmt.h>

namespace routing
{

class Repeat
{
public:
    Repeat(int times, std::chrono::microseconds pause)
        : m_times(times), m_pause(pause)
    {
        m_logger = routing::get_default_logger("Repeat");
    }

protected:
    template <typename FUNCTOR>
    void repeat(FUNCTOR functor)
    {
        int print_period = m_times / 10;

        for (int i = 0; i < m_times; ++i)
        {
            functor();

            sleep_or_busy_spin(m_pause);

            if (i > 0 && print_period > 1000 && i % print_period == 0)
            {
                m_logger->info("[{}/{}]", i, m_times);
            }
        }
    }

private:
    int m_times;
    std::chrono::microseconds m_pause;

    Logger_t m_logger;
};

class Repeat_until
{
public:
    Repeat_until(
        std::chrono::microseconds timeout, std::chrono::microseconds pause)
        : m_timeout(timeout), m_pause(pause)
    {
    }

protected:
    template <typename FUNCTOR>
    void repeat_until(FUNCTOR functor)
    {
        auto start = std::chrono::system_clock::now();

        while (std::chrono::system_clock::now() - start < m_timeout)
        {
            functor();

            sleep_or_busy_spin(m_pause);
        }
    }

private:
    std::chrono::microseconds m_timeout;
    std::chrono::microseconds m_pause;
};

}  // namespace routing

#endif

