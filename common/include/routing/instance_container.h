
#ifndef ROUTING_INSTANCE_CONTAINER_H
#define ROUTING_INSTANCE_CONTAINER_H

#include <routing/stdext.h>

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <memory>
#include <vector>

#include <spdlog/fmt/fmt.h>

namespace routing
{
namespace detail
{
template <typename T>
std::intptr_t
to_handle(T* ptr)
{
    return reinterpret_cast<std::intptr_t>(ptr);
}

template <typename T>
T*
from_handle(std::intptr_t handle)
{
    return reinterpret_cast<T*>(handle);
}

template <typename T>
std::intptr_t
to_handle(std::unique_ptr<T> const& ptr)
{
    return to_handle(ptr.get());
}
}  // namespace detail

template <typename T, bool DEBUG = false>
class Instance_container
{
public:
    using value_t  = std::unique_ptr<T>;
    using handle_t = std::intptr_t;

    Instance_container(std::function<void()> const& on_error)
    {
        if (DEBUG)
        {
            m_on_error = on_error;
        }
    }

    template <typename... ARGS>
    handle_t add(ARGS&&... args)
    {
        m_instances.emplace_back(
            routing::make_unique<T>(std::forward<ARGS>(args)...));

        T* instance_ptr = m_instances.back().get();

        return detail::to_handle(instance_ptr);
    }

    value_t remove(handle_t handle)
    {
        auto it = get_it(handle);

        if (it == m_instances.end())
        {
            if (DEBUG)
            {
                m_on_error();
            }

            return {};
        }

        value_t removed_value = std::move(*it);

        m_instances.erase(it);

        return removed_value;
    }

    T* get_instance(handle_t handle) const
    {
        if (DEBUG)
        {
            auto it = get_it(handle);

            if (it == m_instances.end())
            {
                m_on_error();
            }
        }

        return detail::from_handle<T>(handle);
    }

    void remove_all() { m_instances.clear(); }

    std::size_t size() const { return m_instances.size(); }

private:
    std::vector<std::unique_ptr<T>> m_instances;
    std::function<void()> m_on_error;

    auto get_it(handle_t handle) const -> decltype(m_instances.begin())
    {
        return std::find_if(
            m_instances.begin(), m_instances.end(),
            [handle](value_t const& val) {
                return detail::to_handle(val) == handle;
            });
    }

    auto get_it(handle_t handle) -> decltype(m_instances.begin())
    {
        return std::find_if(
            m_instances.begin(), m_instances.end(),
            [handle](value_t const& val) {
                return detail::to_handle(val) == handle;
            });
    }
};

}  // namespace routing

#endif
