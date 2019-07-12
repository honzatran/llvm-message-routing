
#include <routing/buffer.h>
#include <gtest/gtest.h>
#include <iostream>

using namespace routing;
using namespace std;

constexpr int buffer_size = 8192;

class BufferTest : public ::testing::Test
{
protected:
    Buffer buffer = Buffer(8192);
};


TEST_F(BufferTest, position_remaining_test)
{
    EXPECT_EQ(buffer_size, buffer.remaining());

    buffer.set_position(42);
    EXPECT_EQ(buffer_size - 42, buffer.remaining());

    buffer.set_position(8192);
    EXPECT_EQ(0, buffer.remaining());
}

TEST_F(BufferTest, create_buffer_view_test)
{
    auto buffer_view = buffer.slice_from_position(128);

    EXPECT_EQ(128, buffer_view.get_length());
}

TEST(BufferViewTest, accept_test)
{
    std::vector<int> data(1024);

    std::size_t sizes[] = { 100, 200, 300, 1024 };

    std::size_t length_position = 0;
    for (std::size_t i = 0; i < 4; i++)
    {
        data[length_position] = sizes[i] - 1;
        length_position += sizes[i];
    }

    Buffer_view view{&data[0], data.size()};

    std::size_t i = 0;

    view.accept(
        [&i, sizes](Buffer_view view) {
            EXPECT_EQ(sizes[i], view.get_length() / sizeof(int));

            i++;
        },
        [](Buffer_view view) -> int {
            int length = view.as_value<int>(0);

            if (length * sizeof(int) > view.get_length())
            {
                return -1;
            }

            return sizeof(int) * length + 4;
        });

    EXPECT_EQ(3, i);
}


