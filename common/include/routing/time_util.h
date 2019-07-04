#ifndef ROUTING_TIME_UTIL_H
#define ROUTING_TIME_UTIL_H

#include <cstdint>
#include <thread>

namespace routing
{
inline void
busy_spin(std::chrono::microseconds pause)
{
    auto now = std::chrono::system_clock::now();

    while (std::chrono::system_clock::now() - now < pause)
    {
    }
}

inline void
sleep_or_busy_spin(std::chrono::microseconds pause)
{
    if (pause < std::chrono::milliseconds{1})
    {
        busy_spin(pause);
    }
    else
    {
        std::this_thread::sleep_for(pause);
    }
}

inline std::uint64_t
rdtscp()
{
    std::uint64_t rax, rdx;
    asm volatile("rdtscp\n" : "=a"(rax), "=d"(rdx) : "a"(0) :);

    return (rdx << 32) | rax;
};
}  // namespace routing

#endif
