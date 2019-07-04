

#ifndef ROUTING_RESOURCE_STATE_H
#define ROUTING_RESOURCE_STATE_H

#include "hooks.h"
#include <routing/logger.h>

#include <atomic>
#include <spdlog/fmt/fmt.h>


namespace routing
{
class Resource_state : public Class_hook_guard<Resource_state>
{
public:
    Resource_state(std::string const& name, bool enabled = true)
        : hook_base_t(fmt::format("Resource_state_{}", name)), m_name(name)
    {
        m_enabled.store(enabled);

        std::string resource_state_name
            = fmt::format("Resource_state_{}", name);

        m_logger = routing::get_default_logger("Resource_state");
    }

    Resource_state() 
    {
        m_logger = routing::get_default_logger("Resource_state");
    }

    Resource_state(Resource_state const& other) = delete;
    Resource_state& operator=(Resource_state const& other) = delete;

    Resource_state(Resource_state&& other)
        : hook_base_t(std::move(other)),
          m_name(std::move(other.m_name))
    {
        m_enabled.store(other.m_enabled);
        other.m_enabled.store(false);
    }

    Resource_state& operator=(Resource_state && other) 
    {
        if (this != &other)
        {
            hook_base_t::operator=(std::move(other));

            m_enabled.store(other.m_enabled);
            other.m_enabled.store(false);
        }

        return *this;
    }

    void disable()
    {
        m_enabled.store(false, std::memory_order_relaxed);
    }

    bool is_enabled() const
    {
        return m_enabled.load(std::memory_order_relaxed);
    }

    void execute_hook_impl()
    {
        disable();
    }

private:
    std::atomic_bool m_enabled;
    std::string m_name = "";

    Logger_t m_logger;
};


}

#endif
