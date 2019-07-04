
#include <routing/hooks.h>
#include <gtest/gtest.h>

using namespace routing;
using namespace std;

class HooksTest : public ::testing::Test {
protected:
    void TearDown() override { remove_all_hooks(); }
};


TEST_F(HooksTest, add_hooks_test)
{
    int i = 0;

    add_interrupt_hook(string("test1"), [&i]() { i += 1; });

    execute_all_hooks();

    EXPECT_EQ(1, i);
    
    i = 0;
    
    add_interrupt_hook(string("test2"), [&i]() { i += 2; });
    add_interrupt_hook(string("test3"), [&i]() { i += 3; });

    execute_all_hooks();

    EXPECT_EQ(6, i);
};



TEST_F(HooksTest, remove_hook_test)
{

    int i = 0;

    add_interrupt_hook(string("test1"), [&i]() { i += 4; });
    add_interrupt_hook(string("test2"), [&i]() { i += 5; });
    
    execute_all_hooks();

    EXPECT_EQ(9, i);
    
    i = 0;
    
    remove_hook(string("test1"));

    execute_all_hooks();

    EXPECT_EQ(5, i);

    i = 0;
    
    remove_hook(string("test1"));
    remove_hook(string("test2"));
    
    execute_all_hooks(); 

    EXPECT_EQ(0, i);
};


