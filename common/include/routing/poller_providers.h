

#ifndef ROUTING_POLLER_PROVIDERS_H
#define ROUTING_POLLER_PROVIDERS_H

#include <routing/poller.h>
#include <routing/config.h>
#include <routing/idle_strategy.h>
#include <routing/synchronized.h>
#include <routing/stdext.h>

namespace routing
{

program_options::options_description
get_poller_provider_options(std::string const& name);

struct Poller_provider_options
{
    std::size_t poller_count;
    std::size_t poller_capacity;
    std::size_t poller_insert_capacity;
};

Poller_provider_options
extract_poller_options(Config const& config, std::string const& name);

template <typename POLLABLE, typename IDLE_STRATEGY = Noop_idle_strategy>
class Round_robin_poller_provider
{
public:
    using Poller_t = Poller<POLLABLE, IDLE_STRATEGY>;

    Round_robin_poller_provider(
        std::string const& name,
        IThread_factory* thread_factory,
        std::size_t poller_count,
        std::size_t poller_capacity,
        std::size_t poller_insert_capacity)
        : m_next_poller_index(0)
    {
        for (std::size_t i = 0; i < poller_count; ++i)
        {
            m_pollers.push_back(std::make_unique<Poller_t>(
                fmt::format("{}_{}", name, i),
                poller_capacity,
                poller_insert_capacity));

            m_pollers[i]->start(thread_factory);
        }
    }

    Round_robin_poller_provider(
        std::string const& name,
        IThread_factory* thread_factory,
        Poller_provider_options const& provider_options)
        : Round_robin_poller_provider(
              name,
              thread_factory,
              provider_options.poller_count,
              provider_options.poller_capacity,
              provider_options.poller_insert_capacity)
    {
    }

    ~Round_robin_poller_provider()
    {
        stop();
    }

    void stop()
    {
        for (auto& poller : m_pollers)
        {
            poller->stop();
        }
    }

    Poller_t& get_next_poller()
    {
        auto locked = m_next_poller_index.lock();
        std::size_t& next = *locked;

        return *m_pollers[next++ % m_pollers.size()];
    }
    
private:
    std::vector<std::unique_ptr<Poller_t>> m_pollers;

    Synchronized<std::size_t> m_next_poller_index;
};

template <typename POLLER>
using Round_robin_poller_provider_t = Round_robin_poller_provider<
    typename POLLER::Pollable_t,
    typename POLLER::Idle_strategy_t>;
};

#endif
