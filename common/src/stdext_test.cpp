

#include <routing/stdext.h>
#include <gtest/gtest.h>


using namespace routing;


bool is_negative(int v) 
{
    return v < 0;
}

void test_is_invalid(Function_ref<bool(int)> empty_function)
{
    EXPECT_FALSE(empty_function);
}


TEST(Function_ref_test, simple_fn_test)
{
    Function_ref<bool(int)> function_ref(is_negative);

    EXPECT_TRUE(function_ref(-5));
    EXPECT_FALSE(function_ref(5));
}

TEST(Function_ref, lambda_test)
{
    auto is_negative_lambda = [] (int num) { return num < 0; };

    Function_ref<bool(int)> function_ref(is_negative_lambda);

    EXPECT_TRUE(function_ref(-5));
    EXPECT_FALSE(function_ref(5));
}

TEST(Function_ref, bool_operator_test)
{
    Function_ref<bool(int)> empty_function;
    EXPECT_FALSE(empty_function);

    test_is_invalid(empty_function);
    
    Function_ref<bool(int)> initialized_function(is_negative);
    EXPECT_TRUE(initialized_function);
}

TEST(Function_ref, void_lambda_test)
{
    int i = 0;
    auto void_lambda = [&i] (int delta) { i += delta; };

    Function_ref<void(int)> increase_fn(void_lambda);

    increase_fn(42);

    EXPECT_EQ(42, i);

    int k = 0;
    auto void_lambda_1 = [&k] () { k += 42; };

    Function_ref<void()> increase_one_fn(void_lambda_1);

    increase_one_fn();

    Function_ref<void()> tmp(increase_one_fn);
    tmp();

    EXPECT_EQ(84, k);
}

void exec_fn_ref(int delta, Function_ref<void(int)> ref)
{
    ref(delta);
}

TEST(Function_ref, pass_to_other_method)
{
    int i = 0;
    auto void_lambda = [&i] (int delta) { i += delta; };

    Function_ref<void(int)> increase_fn(void_lambda);

    exec_fn_ref(42, increase_fn);

    EXPECT_EQ(42, i);
}

