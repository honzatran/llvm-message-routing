

#include <routing/bits_util.h>
#include <gtest/gtest.h>

TEST(Bit_util_test, high32)
{
    std::uint64_t val = 0x1ULL | (0x42ULL <<32);

    ASSERT_EQ(0x42ULL, routing::high32(val));
}

TEST(Bit_util_test, low32)
{
    std::uint64_t val = 0x42ULL | (0x1ULL <<32);

    ASSERT_EQ(0x42ULL, routing::low32(val));
}
