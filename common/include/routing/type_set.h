

#ifndef ROUTING_TYPED_SET_H
#define ROUTING_TYPED_SET_H

#include <iterator>
#include <utility>

#include "meta.h"

namespace routing
{
template <typename T, typename... ARGS>
class Type_set 
{
    static_assert(is_unique<ARGS...>::value, "ARGS are not unique");
public:

    template <typename U>
    T const& get_element() const
    {
        constexpr std::size_t index = index_of<U, 0, ARGS...>::value;
        static_assert(index < sizeof...(ARGS), "type U is not present in ARGS");

        return m_elements[index];
    }

    template <typename U>
    T& get_element()
    {
        constexpr std::size_t index = index_of<U, 0, ARGS...>::value;
        static_assert(index < sizeof...(ARGS), "type U is not present in ARGS");

        return m_elements[index];
    }

    template <typename U>
    std::size_t get_index()
    {
        constexpr std::size_t index = index_of<U, 0, ARGS...>::value;
        static_assert(index < sizeof...(ARGS), "type U is not present in ARGS");
        return index;
    }

    template <typename U, typename... CONSTRUCTOR_ARGS>
    void emplace(CONSTRUCTOR_ARGS&&... constructor_args)
    {
        std::size_t index = get_index<U>();
        T* ptr = &m_elements[index];

        new(ptr) T(std::forward<CONSTRUCTOR_ARGS>(constructor_args)...);
    }

private:
    T m_elements[sizeof...(ARGS)];

public:
    using value_type             = T;
    using pointer                = T*;
    using const_pointer          = T const*;
    using reference              = T&;
    using const_reference        = T const&;
    using const_iterator         = const_pointer;
    using iterator               = pointer;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using reverse_iterator       = std::reverse_iterator<iterator>;
    using size_type              = std::size_t;
    using difference_type        = std::ptrdiff_t;

    iterator begin() noexcept { return std::begin(m_elements); }
    iterator end() noexcept { return std::end(m_elements); }
            
    const_iterator cbegin() const noexcept { return std::begin(m_elements); }
    const_iterator cend() const noexcept { return std::end(m_elements); }

    reverse_iterator rbegin() noexcept
    {
        return reverse_iterator(end());
    }

    reverse_iterator rend() noexcept
    {
        return reverse_iterator(begin());
    }

    const_reverse_iterator crbegin() const noexcept
    {
        return const_reverse_iterator(cend());
    }

    reverse_iterator crend() const noexcept
    {
        return const_reverse_iterator(cbegin());
    }
};



}

#endif
