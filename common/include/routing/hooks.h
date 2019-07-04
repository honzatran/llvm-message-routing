#ifndef ROUTING_HOOKS_H
#define ROUTING_HOOKS_H

#include <boost/optional.hpp>
#include <functional>

namespace routing
{
void
add_interrupt_hook(
    std::string const& hook_name, std::function<void()> const& hook);

void
remove_hook(std::string const& hook_name);

void
execute_all_hooks();

void
execute_and_remove_all_hooks();

void
remove_all_hooks();

template <typename T>
class Class_hook_guard
{
public:
    using hook_base_t = Class_hook_guard<T>;

    Class_hook_guard(Class_hook_guard const& other) = delete;
    Class_hook_guard& operator=(Class_hook_guard const& other) = delete;

    Class_hook_guard(Class_hook_guard&& other)
        : m_hook_name(std::move(other.m_hook_name))
    {
        other.remove_hook_if_set();
        other.m_hook_name = boost::none;
        
        if (m_hook_name)
        {
            add_interrupt_hook(*m_hook_name, [this] { execute_hook(); });
        }
    }

    Class_hook_guard& operator=(Class_hook_guard&& other)
    {
        if (this != &other)
        {
            remove_hook_if_set();
            m_hook_name = std::move(other.m_hook_name);

            other.remove_hook_if_set();
            other.m_hook_name = boost::none;

            if (m_hook_name)
            {
                add_interrupt_hook(*m_hook_name, [this] { execute_hook(); });
            }
        }

        return *this;
    }

    ~Class_hook_guard() { remove_hook_if_set(); }

private:
    Class_hook_guard(
        std::string const& hook_name)
        : m_hook_name(hook_name)
    {
        add_interrupt_hook(hook_name, [this] { execute_hook(); });
    }

    Class_hook_guard() : m_hook_name(boost::none) {}

    friend T;

    boost::optional<std::string> m_hook_name;

    void execute_hook()
    {
        get_instance().execute_hook_impl();
    }

    void remove_hook_if_set()
    {
        if (m_hook_name)
        {
            remove_hook(m_hook_name.get());
        }
    }

    T& get_instance()
    {
        return static_cast<T&>(*this);
    }
};

class Hook_guard
{
public:
    Hook_guard() : m_hook_name(boost::none) {}

    Hook_guard(std::string const& hook_name, std::function<void()> const& hook)
        : m_hook_name(hook_name)
    {
        add_interrupt_hook(hook_name, hook);
    }

    Hook_guard(Hook_guard const& other) = delete;
    Hook_guard& operator=(Hook_guard const& other) = delete;

    void init(std::string const& hook_name, std::function<void()> const& hook)
    {
        remove_hook_if_set();

        m_hook_name = hook_name;
        add_interrupt_hook(hook_name, hook);
    }

    Hook_guard(Hook_guard&& other)
    {
        if (this != &other)
        {
            remove_hook_if_set();
            m_hook_name       = other.m_hook_name;

            other.remove_hook_if_set();
            other.m_hook_name = boost::none;
        }
    }

    Hook_guard& operator=(Hook_guard&& other)
    {
        if (this != &other)
        {
            remove_hook_if_set();
            m_hook_name       = other.m_hook_name;

            other.remove_hook_if_set();
            other.m_hook_name = boost::none;
        }

        return *this;
    }

    ~Hook_guard() { remove_hook_if_set(); }
private:
    boost::optional<std::string> m_hook_name;

    void remove_hook_if_set()
    {
        if (m_hook_name)
        {
            remove_hook(m_hook_name.get());
        }
    }
};
};

#endif
