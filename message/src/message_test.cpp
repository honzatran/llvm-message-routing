

#include <routing/message/message.h>
#include <routing/slab_allocator.h>
#include <routing/stdext.h>

#include <gtest/gtest.h>
#include <memory>


using namespace routing;

class Message_test : public ::testing::Test
{
public:

    void SetUp() override
    {
        auto slab_allocator = std::make_unique<Slab_allocator_t>(Malloc_allocator());
        m_tested_message = engine::Message(std::move(slab_allocator));
    }

protected:
    engine::Message m_tested_message;
};

TEST_F(Message_test, init_capacity)
{
    ASSERT_EQ(32, m_tested_message.capacity());
}

TEST_F(Message_test, empty)
{
    ASSERT_TRUE(m_tested_message.empty());
}

TEST_F(Message_test, basic_access)
{
    ASSERT_FALSE(m_tested_message.has_int(42));

    m_tested_message.set_int(42, 42);

    ASSERT_TRUE(m_tested_message.has_int(42));

    ASSERT_EQ(42, m_tested_message.get_int(42));
}

TEST_F(Message_test, resize)
{
    for (int i = 0; i < 32; i++)
    {
        ASSERT_FALSE(m_tested_message.has_int(i));

        m_tested_message.set_int(i, i);

        ASSERT_TRUE(m_tested_message.has_int(i)) << i;

        ASSERT_EQ(i, m_tested_message.get_int(i));

        ASSERT_EQ(i + 1, m_tested_message.size());
    }

    int resize_key = 32;

    m_tested_message.set_int(resize_key, resize_key);

    ASSERT_EQ(resize_key, m_tested_message.get_int(resize_key));
    ASSERT_TRUE(m_tested_message.has_int(resize_key));

    ASSERT_EQ(33, m_tested_message.size());

    for (int i = 0; i < 33; i++)
    {
        EXPECT_TRUE(m_tested_message.has_int(i)) << i;
        EXPECT_EQ(i, m_tested_message.get_int(i));
    }
}

TEST_F(Message_test, insert_int_1000_000)
{
    for (int i = 0; i < 1'000'000; i++)
    {
        ASSERT_FALSE(m_tested_message.has_int(i)) << i;

        m_tested_message.set_int(i, i);

        ASSERT_TRUE(m_tested_message.has_int(i)) << i;

        ASSERT_EQ(i, m_tested_message.get_int(i));

        ASSERT_EQ(i + 1, m_tested_message.size());
    }
}

TEST_F(Message_test, insert_long_1000_000)
{
    for (std::int64_t i = 0; i < 1'000'000; i++)
    {
        ASSERT_FALSE(m_tested_message.has_long(i)) << i;

        m_tested_message.set_long(i, i);

        ASSERT_TRUE(m_tested_message.has_long(i)) << i;

        ASSERT_EQ(i, m_tested_message.get_long(i));

        ASSERT_EQ(i + 1, m_tested_message.size());
    }
}

TEST_F(Message_test, insert_double_1000_000)
{
    for (std::int64_t i = 0; i < 1'000'000; i++)
    {
        ASSERT_FALSE(m_tested_message.has_double(i)) << i;

        m_tested_message.set_double(i, i);

        ASSERT_TRUE(m_tested_message.has_double(i)) << i;

        ASSERT_DOUBLE_EQ(i, m_tested_message.get_double(i));

        ASSERT_EQ(i + 1, m_tested_message.size());
    }
}

