

#ifndef ROUTING_SYNCHRONIZED_H
#define ROUTING_SYNCHRONIZED_H

#include <mutex>
#include <type_traits>

namespace routing
{
namespace detail
{
template <typename SYNCHRONIZED_TYPE>
using Synchronized_data_type =
    typename std::conditional<std::is_const<SYNCHRONIZED_TYPE>::value,
                              typename SYNCHRONIZED_TYPE::Data_type const,
                              typename SYNCHRONIZED_TYPE::Data_type>::type;
}

template <typename SYNCHRONIZED_TYPE>
class Locked_ptr_base;
template <typename SYNCHRONIZED_TYPE>
class Locked_ptr;

template <typename T>
class Synchronized
{
private:
    class Not_implemented_type;

public:
    using Data_type          = T;
    using Locked_ptr_t       = ::routing::Locked_ptr<Synchronized>;
    using Const_locked_ptr_t = ::routing::Locked_ptr<const Synchronized>;

    Synchronized() = default;

    // no need to take a lock since the instance is not created yet
    explicit Synchronized(T const& val) : m_val(val) {}
    explicit Synchronized(T&& val) : m_val(std::move(val)) {}
    Synchronized(
        typename std::conditional<std::is_copy_constructible<T>::value,
                                  Synchronized const&,
                                  Not_implemented_type>::type other)
    {
    }

    Synchronized& operator=(
        typename std::conditional<std::is_copy_assignable<T>::value,
                                  Synchronized const&,
                                  Not_implemented_type>::type other)
    {
        if (this == &other)
        {
        }
        else if (this < &other)
        {
            auto guard1 = lock();
            auto guard2 = other.lock();

            m_val = other.m_val;
        }
        else
        {
            auto guard1 = other.lock();
            auto guard2 = lock();

            m_val = other.m_val;
        }

        return *this;
    }

    Synchronized(Synchronized&& other) : Synchronized(other, other.lock()) {}
    Synchronized& operator=(Synchronized&& other)
    {
        if (this == &other)
        {
        }
        else if (this < &other)
        {
            auto guard1 = lock();
            auto guard2 = other.lock();

            m_val = std::move(other.m_val);
        }
        else
        {
            auto guard1 = other.lock();
            auto guard2 = lock();

            m_val = std::move(other.m_val);
        }

        return *this;
    }

    Synchronized& operator=(T const& val)
    {
        auto guard = lock();
        m_val      = val;

        return *this;
    }

    Synchronized& operator=(T&& val)
    {
        auto guard = lock();
        m_val      = std::move(val);

        return *this;
    }

    template <typename F>
    void with_lock(F&& f)
    {
        auto guard = lock();
        f(m_val);
    }

    Locked_ptr_t lock() { return Locked_ptr_t(this); }
    Const_locked_ptr_t lock() const { return Const_locked_ptr_t(this); }
    T copy() const
    {
        auto guard = lock();

        return m_val;
    }

private:
    template <typename SYNCHRONIZED_TYPE>
    friend class Locked_ptr_base;

    template <typename SYNCHRONIZED_TYPE>
    friend class Locked_ptr;

    mutable std::mutex m_mutex;

    T m_val;

    Synchronized(Synchronized const& other, Locked_ptr_t const& /*unused*/)
        : m_val(other.m_val)
    {
    }

    Synchronized(Synchronized&& other, Locked_ptr_t const& /*unused*/)
        : m_val(std::move(other))
    {
    }
};

template <typename SYNCHRONIZED_TYPE>
class Locked_ptr_base
{
public:
    ~Locked_ptr_base() = default;
    Locked_ptr_base(Locked_ptr_base&& other)
        : m_lock(std::move(other.m_lock)), m_parent(other.m_parent)
    {
        other.m_parent = nullptr;
    }

    Locked_ptr_base& operator=(Locked_ptr_base&& other)
    {
        m_lock         = std::move(other.m_lock);
        m_parent       = other.m_parent;
        other.m_parent = nullptr;

        return *this;
    }

protected:
    Locked_ptr_base() = default;
    explicit Locked_ptr_base(SYNCHRONIZED_TYPE* parent)
        : m_lock(parent->m_mutex), m_parent(parent)
    {
    }

    std::unique_lock<std::mutex> m_lock;

    SYNCHRONIZED_TYPE* m_parent;
};

template <typename SYNCHRONIZED_TYPE>
class Locked_ptr : protected Locked_ptr_base<SYNCHRONIZED_TYPE>
{
private:
    using Base_t     = Locked_ptr_base<SYNCHRONIZED_TYPE>;
    using CData_type = detail::Synchronized_data_type<SYNCHRONIZED_TYPE>;

public:
    Locked_ptr() = default;
    explicit Locked_ptr(SYNCHRONIZED_TYPE* parent) : Base_t(parent) {}
    Locked_ptr(Locked_ptr const& other) = delete;
    Locked_ptr& operator=(Locked_ptr const& other) = delete;

    Locked_ptr(Locked_ptr&& other) = default;
    Locked_ptr& operator=(Locked_ptr&& other) = default;

    ~Locked_ptr() = default;
    bool is_null() const { return this->m_parent == nullptr; }
    explicit operator bool() const { return this->m_parent != nullptr; }
    CData_type* operator->() const { return &(this->m_parent->m_val); }
    CData_type& operator*() const { return this->m_parent->m_val; }
private:
};
}

#endif
