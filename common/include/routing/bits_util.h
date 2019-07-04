
#ifndef ROUTING_BITS_UTIL_H
#define ROUTING_BITS_UTIL_H

#include <cstdint>

namespace routing
{

inline std::uint32_t high32(std::uintptr_t addr)
{
    return (addr >> 32) & 0xFFFFFFFF;
}

inline std::uint32_t low32(std::uintptr_t addr)
{
    return addr & 0xFFFFFFFF;
}

inline std::int64_t combine_to_int64(std::int32_t a, std::int32_t b)
{
    return (((std::int64_t) a << 32) | (std::int64_t) b);
}

inline std::int64_t shift_by32(std::int32_t value)
{
    return (((std::int64_t) value) << 32);
}

}


#endif
