

#include <routing/bits_util.h>
#include <routing/logger.h>

#include <cstdint>

__attribute__((annotate("ENGINE")))
std::int64_t external_bits_util(int a, int b)
{
    return routing::combine_to_int64(a, b);
}
