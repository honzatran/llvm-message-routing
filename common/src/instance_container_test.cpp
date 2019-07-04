
#include <routing/instance_container.h>

#include <routing/call_counter.h>
#include <routing/testing.h>

#include <gtest/gtest.h>

using namespace routing;

class Instance_container_test : public ::testing::Test
{
public:

    struct Mock
    {
        Mock() = default;
        Mock(int a, int b) : m_a(a), m_b(b) {}
        int m_a{42};
        int m_b{43};
    };
};

TEST_F(Instance_container_test, add_get)
{
    Instance_container<Mock> test_container([] {});

    std::intptr_t handle = test_container.add();

    Mock* mock = test_container.get_instance(handle);
    
    ASSERT_TRUE(mock);

    EXPECT_EQ(42, mock->m_a);
    EXPECT_EQ(43, mock->m_b);

    std::vector<std::intptr_t> handles;

    for (int i = 0; i < 1000; ++i)
    {
        std::intptr_t handle = test_container.add(i, i);

        handles.push_back(handle);
    }

    EXPECT_EQ(1001, test_container.size());

    int i = 0;
    for (std::intptr_t handle : handles)
    {
        Mock* mock = test_container.get_instance(handle);

        EXPECT_EQ(i, mock->m_a);
        EXPECT_EQ(i, mock->m_b);

        i++;
    }
}

TEST_F(Instance_container_test, add_remove)
{
    Instance_container<Mock> test_container([] {});

    std::intptr_t handle = test_container.add();

    Mock* mock = test_container.get_instance(handle);
    
    ASSERT_TRUE(mock);

    EXPECT_EQ(1, test_container.size());

    test_container.remove(handle);

    EXPECT_EQ(0, test_container.size());

    std::vector<std::intptr_t> handles;

    for (int i = 0; i < 1000; ++i)
    {
        std::intptr_t handle = test_container.add(i, i);

        handles.push_back(handle);
    }

    EXPECT_EQ(1000, test_container.size());

    int size = 1000;

    for (std::intptr_t handle : handles)
    {
        EXPECT_EQ(size, test_container.size());
        test_container.remove(handle);

        size--;
    }

    EXPECT_EQ(0, test_container.size());
}

TEST_F(Instance_container_test, get_invalid)
{
    auto on_error  
        = Called_exectly_void<>(1, DEFINED("get invalid"));

    Instance_container<Mock, true> test_container([&on_error] { on_error(); });

    int handle = test_container.add(1, 2);

    test_container.get_instance(handle - 1);
}

TEST_F(Instance_container_test, remove_invalid)
{
    auto on_error  
        = Called_exectly_void<>(1, DEFINED("remove invalid"));

    Instance_container<Mock, true> test_container([&on_error] { on_error(); });

    int handle = test_container.add(1, 2);

    test_container.remove(handle - 1);
}






