
#ifndef ROUTING_ENUM_UTILS_H
#define ROUTING_ENUM_UTILS_H

#include <type_traits>

namespace routing
{
template <typename E>
constexpr auto
to_integral(E value) -> typename std::underlying_type<E>::type
{
    return static_cast<typename std::underlying_type<E>::type>(value);
}

}  // namespace routing

#endif
