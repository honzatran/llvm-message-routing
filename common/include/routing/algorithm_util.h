
#ifndef ROUTING_ALGORITHM_UTIL_H
#define ROUTING_ALGORITHM_UTIL_H

#include <boost/optional.hpp>
#include <routing/stdext.h>
#include <algorithm>

namespace routing
{

template <typename T>
boost::optional<std::size_t>
find_in_small_sorted_sequence(T elem, std::vector<T> const& sorted_sequence)
{
    auto it = std::find(
            sorted_sequence.begin(),
            sorted_sequence.end(),
            elem);

    if (it == sorted_sequence.end())
    {
        return {};
    }

    return std::distance(sorted_sequence.begin(), it);
}

template <typename T>
boost::optional<std::size_t>
find_in_large_sorted_sequence(T elem, std::vector<T> const& sorted_sequence)
{
    auto it = std::lower_bound(
            sorted_sequence.begin(),
            sorted_sequence.end(),
            elem);

    if (it == sorted_sequence.end() 
        || *it != elem)
    {
        return {};
    }

    return std::distance(sorted_sequence.begin(), it);
}

template <typename T>
boost::optional<std::size_t>
find_in_sorted_sequence(T elem, std::vector<T> const& sorted_sequence)
{
    if (sorted_sequence.size() <= 8)
    {
        return find_in_small_sorted_sequence(elem, sorted_sequence);
    }

    return find_in_large_sorted_sequence(elem, sorted_sequence);
}

template <typename T, typename PREDICATE>
boost::optional<T>
find_if(
    PREDICATE predicate,
    std::vector<T> const& elements)
{
    auto it = std::find_if(elements.begin(), elements.end(), predicate);

    if (it != elements.end())
    {
        return *it;
    }
    else 
    {
        return {}; 
    }
}

}  // namespace routing

#endif
