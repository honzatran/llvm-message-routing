

#ifndef ROUTING_RANDOM_UTILS_H
#define ROUTING_RANDOM_UTILS_H

#include <routing/logger.h>

#include <string>
#include <random>

namespace routing
{
template <typename INT_TYPE>
class Random_int_generator
{
public:
    Random_int_generator(std::string const& name)
    {
        m_logger = routing::get_default_logger(
            fmt::format("Random_generator.{}", name));

        static std::random_device rd;
        unsigned seed = rd();
        m_engine = std::mt19937(seed);

        m_logger->info("Random int generator created with seed {}.", seed);
    }

    INT_TYPE operator() ()
    {
        std::uniform_int_distribution<INT_TYPE> distribution;
        return distribution(m_engine);
    }

    INT_TYPE operator() (INT_TYPE a, INT_TYPE b)
    {
        std::uniform_int_distribution<INT_TYPE> distribution(a, b);
        return distribution(m_engine);
    }

private:
    std::mt19937 m_engine;

    Logger_t m_logger;
};

}  // namespace routing

#endif
