

#include <routing/call_counter.h>
#include <routing/debug.h>
#include <routing/testing.h>

#include <gtest/gtest.h>

using namespace std;
using namespace routing;

TEST(Once_in_time_test, test)
{
    auto call_counter = define_void_mock<>(2, DEFINED("once in time"));

    auto counter_fn = [&call_counter] { call_counter(); };

    Once_in_time once_in_time(chrono::milliseconds{10});

    auto now = chrono::system_clock::now();

    while (chrono::system_clock::now() - now < chrono::milliseconds{22})
    {
        once_in_time(counter_fn);
    }
}
