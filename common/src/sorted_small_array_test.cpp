

#include <gtest/gtest.h>
#include <routing/sorted_small_array.h>

using namespace routing;

class Sorted_small_array_test : public ::testing::Test
{
public:
    Sorted_small_array<int, 8> get_find_lower_test_input() const 
    {
        Sorted_small_array<int, 8> small_array(0);

        small_array[0] = 1;
        small_array[1] = 1;
        small_array[2] = 2;
        small_array[3] = 500;
        small_array[4] = 1000;
        small_array[5] = 2000;
        small_array[6] = 4000;
        small_array[7] = 8000;
    
        return small_array;
    }
};

TEST_F(Sorted_small_array_test, after_construction_filled_with_min)
{
    Sorted_small_array<int, 8> small_array(0);

    ASSERT_EQ(small_array[0], 0);
}

TEST_F(Sorted_small_array_test, shift_and_insert_single)
{
    Sorted_small_array<int, 8> small_array(0);

    small_array.shift_and_insert(1);

    for (std::size_t i = 0; i < 7; ++i)
    {
        ASSERT_EQ(0, small_array[i]);
    }

    ASSERT_EQ(1, small_array[7]);
}

TEST_F(Sorted_small_array_test, shift_and_insert_whole_array)
{
    Sorted_small_array<int, 8> small_array(-1);

    for (std::size_t i = 0; i < 8; ++i)
    {
        small_array.shift_and_insert(i);
    }

    for (std::size_t i = 0; i < 8; ++i)
    {
        ASSERT_EQ(i, small_array[i]);
    }

    small_array.shift_and_insert(42);

    for (std::size_t i = 0; i < 7; ++i)
    {
        ASSERT_EQ(i + 1, small_array[i]);
    }

    ASSERT_EQ(42, small_array[7]);
}

TEST_F(Sorted_small_array_test, max)
{
    Sorted_small_array<int, 8> small_array(-1);

    for (std::size_t i = 0; i < 8; ++i)
    {
        small_array.shift_and_insert(i + 42);
    }

    ASSERT_EQ(7 + 42, small_array.max());
}

TEST_F(Sorted_small_array_test, min)
{
    Sorted_small_array<int, 8> small_array(-1);

    for (std::size_t i = 0; i < 8; ++i)
    {
        small_array.shift_and_insert(i + 42);
    }

    ASSERT_EQ(42, small_array.min());
}

TEST_F(Sorted_small_array_test, lower_bound)
{
    auto small_array = get_find_lower_test_input();

    ASSERT_EQ(0, small_array.lower_bound(0));

    ASSERT_EQ(0, small_array.lower_bound(1));

    ASSERT_EQ(2, small_array.lower_bound(2));

    ASSERT_EQ(3, small_array.lower_bound(100));
    ASSERT_EQ(3, small_array.lower_bound(500));
    ASSERT_EQ(4, small_array.lower_bound(501));

    ASSERT_EQ(7, small_array.lower_bound(8000));
}

TEST_F(Sorted_small_array_test, upper_bound)
{
    auto small_array = get_find_lower_test_input();

    ASSERT_EQ(0, small_array.upper_bound(0));

    ASSERT_EQ(2, small_array.upper_bound(1));

    ASSERT_EQ(3, small_array.upper_bound(2));

    ASSERT_EQ(3, small_array.upper_bound(100));
    ASSERT_EQ(4, small_array.upper_bound(500));
    ASSERT_EQ(4, small_array.upper_bound(501));

    ASSERT_EQ(8, small_array.upper_bound(8000));
}

