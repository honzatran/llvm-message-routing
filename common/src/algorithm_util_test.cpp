

#include <gtest/gtest.h>

#include <routing/algorithm_util.h>

using namespace routing;

namespace
{
std::vector<int>
get_test_sorted_sequence(std::size_t size)
{
    std::vector<int> sorted_sequence(size);

    for (int i = 0; i < size; i++)
    {
        if (i < size/2)
        {
            sorted_sequence[i] = i * 2;
        } 
        else
        {
            sorted_sequence[i] = i * 2 + 1;
        }
    }

    return sorted_sequence;
}
}

TEST(Algorithm_util_test, basic_find_in_small)
{
    std::vector<int> sorted_sequence = { 1, 2, 6 };
    auto index = find_in_small_sorted_sequence(6, sorted_sequence);

    ASSERT_TRUE(index);
    ASSERT_EQ(2, *index);
}

TEST(Algorithm_util_test, invalid_find_in_small)
{
    std::vector<int> sorted_sequence = { 1, 2, 6 };
    auto index = find_in_small_sorted_sequence(3, sorted_sequence);

    ASSERT_FALSE(index);
}

TEST(Algorithm_util_test, basic_find_in_large)
{
    std::vector<int> large_sorted_sequence
        = get_test_sorted_sequence(1024 * 1024);

    auto index = find_in_small_sorted_sequence(256, large_sorted_sequence);

    auto index_2 = find_in_small_sorted_sequence(
        (1024 * 1024 - 1) * 2 + 1, large_sorted_sequence);

    ASSERT_TRUE(index);
    ASSERT_EQ(128, *index);

    ASSERT_TRUE(index_2);
    ASSERT_EQ(large_sorted_sequence.size() - 1, *index_2);
}

TEST(Algorithm_util_test, invalid_find_in_large)
{
    std::vector<int> large_sorted_sequence
        = get_test_sorted_sequence(1024 * 1024);

    auto index = find_in_small_sorted_sequence(-1, large_sorted_sequence);

    ASSERT_FALSE(index);

    auto index_1 = find_in_small_sorted_sequence(1, large_sorted_sequence);

    ASSERT_FALSE(index_1);

    auto index_257 = find_in_small_sorted_sequence(257, large_sorted_sequence);

    ASSERT_FALSE(index_257);

    auto index_at_end
        = find_in_small_sorted_sequence(1024 * 1024 * 2, large_sorted_sequence);

    ASSERT_FALSE(index_at_end);

    auto index_behind_end = find_in_small_sorted_sequence(
        1024 * 1024 * 10, large_sorted_sequence);

    ASSERT_FALSE(index_behind_end);
}


