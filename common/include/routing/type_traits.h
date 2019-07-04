

#ifndef ROUTING_TYPE_TRAITS_H
#define ROUTING_TYPE_TRAITS_H

#include "meta.h"
#include "detect.h"

#include <ostream>
#include <routing/fmt.h>

/// This header contains some type meta functions for basic operations
namespace routing
{

namespace detail
{

template <typename T, bool PRINTABLE>
struct Printable
{
};

template <typename T>
struct Printable<T, false>
{
    Printable(T const& ref)  {}

    friend std::ostream& operator<<(
        std::ostream& os,
        Printable<T, false> printer)
    {
        os << "NOT PRINTABLE";
        return os;
    }
};

template <typename T>
struct Printable<T, true>
{
    Printable(T const& ref) : m_ref(ref) {}

    friend std::ostream& operator<<(
        std::ostream& os,
        Printable<T, true> printer)
    {
        os << printer.m_ref;
        return os;
    }

    T const& m_ref;
};


}

template <typename T>
using Is_printable_t
    = decltype(std::declval<std::ostream &>() << std::declval<T&>());

template <typename T>
struct Is_printable
{
    constexpr static bool value = is_detected<Is_printable_t, T>::value;
};

/// Returns either the string value of arg if it implements the operator <<
/// or string NOT PRINTABLE if the arg does not
template <typename ARG>
std::string
to_string_or_not_printable(ARG const& arg)
{
    detail::Printable<ARG, Is_printable<ARG>::value> printer(arg);
    return fmt::format("{}", printer);
}

};

#endif
