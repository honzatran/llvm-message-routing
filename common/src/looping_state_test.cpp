

#include <routing/looping_state.h>
#include <routing/testing.h>
#include <routing/hooks.h>
#include <routing/latches.h>

#include <routing/time_util.h>

#include <gtest/gtest.h>
#include <chrono>
#include <thread>

using namespace routing;
using namespace std;

class Looping_state_test : public ::testing::Test
{
public:
    static void SetUpTestCase()
    {
        auto config_options = 
        {
            get_logger_options_description()
        };

        Config config = 
            routing::parse_args(config_options, TEST_CONFIG_PATH);

        init_logger(config);
    }
};

TEST_F(Looping_state_test, basic_test)
{
    Looping_state<> looping_state("Test");

    EXPECT_FALSE(looping_state.running());

    looping_state.start();

    EXPECT_TRUE(looping_state.running());

    routing::execute_all_hooks();
    looping_state.on_loop_finished();

    EXPECT_FALSE(looping_state.running());
}

TEST_F(Looping_state_test, multithreaded_test)
{
    Looping_state<> looping_state("Test_2");
    Event_latch event_latch;

    EXPECT_FALSE(looping_state.running());

    std::thread running_thread = std::thread([&looping_state, &event_latch] {
        looping_state.start();

        auto start = chrono::system_clock::now();

        event_latch.signal();

        while (looping_state.running()
               && chrono::system_clock::now() - start < chrono::seconds{5})
        {
            sleep_or_busy_spin(chrono::microseconds{100});
        }

        looping_state.on_loop_finished();

        EXPECT_FALSE(looping_state.running());
    });

    EXPECT_EQ(
        std::cv_status::no_timeout, event_latch.wait_for(chrono::seconds{2}));

    routing::execute_all_hooks();

    EXPECT_FALSE(looping_state.running());

    running_thread.join();
}

