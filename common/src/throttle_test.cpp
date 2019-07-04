

#include <routing/throttle.h>
#include <gtest/gtest.h>

#include <chrono>
#include <thread>

using namespace routing;
using namespace std;

namespace 
{
struct Mock_clock
{
    using time_point = chrono::system_clock::time_point;
    
    static std::vector<time_point> timestamps;
    static std::size_t index;

    static time_point now() { return timestamps[index++]; }
};
}

std::size_t Mock_clock::index = 0;
std::vector<Mock_clock::time_point> Mock_clock::timestamps
    = {chrono::system_clock::now(),
       chrono::system_clock::now(),
       chrono::system_clock::now() + chrono::seconds{2}};

TEST(Throttle_test, basic)
{
    Throttle<chrono::system_clock> throttle(2);

    ASSERT_FALSE(throttle.is_full());

    throttle.update();
    ASSERT_FALSE(throttle.crossed());

    throttle.update();
    ASSERT_TRUE(throttle.crossed());
}

TEST(Throttle_test, empty)
{
    Throttle<chrono::system_clock> throttle(0);
    ASSERT_TRUE(throttle.crossed());
}

TEST(Throttle_test, full)
{
    Throttle<chrono::system_clock> throttle(2);

    throttle.update();
    throttle.update();

    ASSERT_TRUE(throttle.is_full());
}

TEST(Throttle_test, update)
{
    Throttle<Mock_clock> throttle(1);

    ASSERT_FALSE(throttle.crossed());

    throttle.update();

    ASSERT_TRUE(throttle.crossed());
}
