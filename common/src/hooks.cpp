#include <routing/hooks.h>
#include <routing/synchronized.h>

#include <routing/logger.h>

#include <algorithm>
#include <vector>
#include <spdlog/fmt/fmt.h>


using namespace routing;
using namespace std;

class Hook_wrapper
{
public:
    Hook_wrapper(
        std::string const& m_hook_name, std::function<void()> const& m_hook)
        : m_hook_name(m_hook_name), m_hook(m_hook)
    {
    }

    void operator()() const {
        m_hook(); 
    }
    std::string name() const { return m_hook_name; }
private:
    std::string m_hook_name;
    std::function<void()> m_hook;
};

bool execute = false;

class Hooks
{
public:
    void add_hook(
        std::string const& hook_name, std::function<void()> const& hook)
    {
        init_logger_guard();
        m_logger->debug("Adding a hook {}", hook_name);

        m_hooks.emplace_back(hook_name, hook);
    }

    void remove_hook(std::string const& hook_name)
    {
        init_logger_guard();
        m_logger->debug("Removing a hook {}", hook_name);

        m_hooks.erase(
            std::remove_if(
                m_hooks.begin(), m_hooks.end(),
                [&hook_name](const Hook_wrapper& wrapper) {
                    return wrapper.name() == hook_name;
                }),
            m_hooks.end());
    }

    void execute()
    {
        init_logger_guard();
        m_logger->debug("Executing hooks");

        for (auto const& hook : m_hooks)
        {
            hook();
        }
    }

    void execute_in_reverse()
    {
        init_logger_guard();
        m_logger->debug("Executing hooks in reverse order");

        std::for_each(
            m_hooks.rbegin(), m_hooks.rend(), [this](Hook_wrapper const& hook) {
                m_logger->debug("Executing hook {}", hook.name());
                hook();
            });

        m_logger->debug("Executing done");
    }

    void remove_all_hooks()
    {
        m_logger->debug("Removing all hooks");
        m_hooks.clear(); 
    }
private:
    Logger_t m_logger = nullptr;
    std::vector<Hook_wrapper> m_hooks;

    void init_logger_guard()
    {
        if (!m_logger)
        {
            m_logger = routing::get_default_logger("Hooks");
        }
    }
    
};

Synchronized<Hooks> hooks;

void
routing::add_interrupt_hook(
    std::string const& hook_name, std::function<void()> const& hook)
{
    auto hooks_locked = hooks.lock();
    hooks_locked->add_hook(hook_name, hook);
};

void
routing::remove_hook(std::string const& hook_name)
{
    if (execute)
    {
        return;
    }

    auto hooks_locked = hooks.lock();
    hooks_locked->remove_hook(hook_name);
};

void
routing::execute_all_hooks()
{
    auto hooks_locked = hooks.lock();
    execute = true;
    hooks_locked->execute_in_reverse();
    execute = false;
}

void
routing::remove_all_hooks()
{
    auto hooks_locked = hooks.lock();
    hooks_locked->remove_all_hooks();
}

void
routing::execute_and_remove_all_hooks()
{
    get_default_logger()->info("Execute and remove all hooks");

    auto hooks_locked = hooks.lock();

    execute = true;
    hooks_locked->execute_in_reverse();
    execute = false;

    hooks_locked->remove_all_hooks();
}

