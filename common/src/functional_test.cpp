

#include <gtest/gtest.h>
#include <routing/functional.h>

using namespace routing;

TEST(Call_once_test, invoke_twice)
{
    int called = 0;

    auto fn = [&called] (int k)
    {
        called += 1;
    };

    routing::Call_once<int> call_once(fn);
    
    // invoke twice
    call_once(2);
    call_once(1);

    // check the call counter, should be one
    ASSERT_EQ(1, called);
}
