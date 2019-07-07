

#include <routing/ring_buffer.h>
#include <routing/call_counter.h>
#include <routing/testing.h>
#include <routing/latches.h>
#include <routing/functional.h>

#include <thread>

#include <gtest/gtest.h>

#include <spdlog/fmt/fmt.h>

using namespace std;
using namespace routing;

class SPSC_ring_buffer_test : public ::testing::Test
{
public:
    class Test_view
    {
    };

private:
};


TEST_F(SPSC_ring_buffer_test, single_thread)
{
    auto on_poll
        = Called_exectly_void<int>(1, DEFINED("single thread"));

    on_poll.expect(1, 100);

    SPSC_ring_buffer ring_buffer(1024);

    std::vector<std::uint8_t> data(100, 42);

    ring_buffer.push(&data[0], 100);

    ring_buffer.poll([&on_poll](Buffer_view buffer_view) {
        on_poll(buffer_view.get_length());
        return buffer_view.get_length();
    });
}

TEST_F(SPSC_ring_buffer_test, multi_thread)
{
    SPSC_ring_buffer ring_buffer(1024);

    auto on_poll
        = Called_exectly_void<int>(10, DEFINED("multi thread"));

    for (int i = 100; i > 90; i--)
    {
        on_poll.expect(1, i);
    }

    Event_latch latch;

    std::thread pusher([&ring_buffer, &latch]() { 

        std::vector<std::uint8_t> data(100, 0);

        for (int i = 100; i > 90; i--)
        {
            data[0] = i;
            ring_buffer.push(&data[0], i);
        }

        latch.signal(); 
    });

    EXPECT_EQ(latch.wait_for(chrono::seconds{2}), cv_status::no_timeout);

    ring_buffer.poll([&on_poll](Buffer_view buffer_view) {
        return buffer_view.accept(
            [&on_poll](Buffer_view view) {
                on_poll(view.get_length());
            },
            [](Buffer_view view) { return view.as_value<std::uint8_t>(0); });
    });

    pusher.join();
}

TEST_F(SPSC_ring_buffer_test, multi_thread_simultanously)
{
    SPSC_ring_buffer ring_buffer(1024);

    auto on_poll
        = Called_exectly_void<int>(10, DEFINED("multi thread"));

    for (int i = 100; i > 90; i--)
    {
        on_poll.expect(1, i);
    }

    Event_latch start_poller_latch, first_polled_latch;

    Event_latch latch;

    std::thread pusher(
        [&ring_buffer, &latch, &start_poller_latch, &first_polled_latch]() {
            std::vector<std::uint8_t> data(100, 0);

            for (int i = 100; i > 90; i--)
            {
                if (i == 95)
                {
                    start_poller_latch.signal();
                }

                data[0] = i;
                ring_buffer.push(&data[0], i);

                if (i == 95)
                {
                    EXPECT_EQ(
                        first_polled_latch.wait_for(chrono::seconds{2}),
                        cv_status::no_timeout);
                }
            }

            latch.signal();
        });

    EXPECT_EQ(
        start_poller_latch.wait_for(chrono::seconds{2}),
        cv_status::no_timeout);

    ring_buffer.poll([&on_poll](Buffer_view buffer_view) {
        return buffer_view.accept(
            [&on_poll](Buffer_view view) { on_poll(view.get_length()); },
            [](Buffer_view view) {
                return view.as_value<std::uint8_t>(0);
            });
    });

    first_polled_latch.signal();

    while (on_poll.called() < 10)
    {
        ring_buffer.poll([&on_poll](Buffer_view buffer_view) {
            return buffer_view.accept(
                [&on_poll](Buffer_view view) { on_poll(view.get_length()); },
                [](Buffer_view view) {
                    return view.as_value<std::uint8_t>(0);
                });
        });
    }

    EXPECT_EQ(latch.wait_for(chrono::seconds{2}), cv_status::no_timeout);

    pusher.join();
}

class Typed_spsc_ring_buffer_test : public ::testing::Test
{
public:
    class Test_content
    {
    };

};

TEST_F(
    Typed_spsc_ring_buffer_test,
    move_into_another_instance)
{
    Typed_spsc_ring_buffer<Test_content, 16> ring_buffer;

    ASSERT_TRUE(ring_buffer.try_push(Test_content()));

    Typed_spsc_ring_buffer<Test_content, 16> moved_ring_buffer(
        std::move(ring_buffer));

    ASSERT_TRUE(moved_ring_buffer.try_poll_one([](Test_content& content) {}));
}

TEST_F(
    Typed_spsc_ring_buffer_test,
    try_push)
{
    // For some reason the capacity must be 3 not 2, this is a
    // behaviour of the boost spsc queue
    Typed_spsc_ring_buffer<Test_content, 2> ring_buffer;

    EXPECT_TRUE(ring_buffer.try_push(Test_content()));
    EXPECT_TRUE(ring_buffer.try_push(Test_content()));

    // now the ring buffer is full
    EXPECT_FALSE(ring_buffer.try_push(Test_content()));
}

TEST_F(
    Typed_spsc_ring_buffer_test,
    force_push)
{
    Typed_spsc_ring_buffer<Test_content, 2> ring_buffer;

    EXPECT_TRUE(ring_buffer.try_push(Test_content()));
    EXPECT_TRUE(ring_buffer.try_push(Test_content()));
    EXPECT_FALSE(ring_buffer.try_push(Test_content()));

    // now the ring buffer is full
    //
    std::thread poller = std::thread([&ring_buffer] {
        std::this_thread::sleep_for(chrono::milliseconds{100});
        EXPECT_TRUE(ring_buffer.try_poll_one([](Test_content conent) {}));
    });

    ring_buffer.force_push(Test_content(), [] {});

    poller.join();
}

TEST_F(
    Typed_spsc_ring_buffer_test,
    push)
{
    Typed_spsc_ring_buffer<Test_content, 2> ring_buffer;

    EXPECT_TRUE(ring_buffer.try_push(Test_content()));
    EXPECT_TRUE(ring_buffer.try_push(Test_content()));
    EXPECT_FALSE(ring_buffer.try_push(Test_content()));

    // now the ring buffer is full
    //
    //
    Event_latch latch;

    auto signal_latch_fn = [&latch] ()
    {
        latch.signal();
    };

    Call_once<> signal_once_latch(signal_latch_fn);

    std::thread poller = std::thread([&ring_buffer, &latch] {
        latch.wait();
        EXPECT_TRUE(ring_buffer.try_poll_one([](Test_content conent) {}));
    });

    ASSERT_TRUE(ring_buffer.push(
        Test_content(), 1000, [&signal_once_latch] { 
            signal_once_latch(); 
            std::this_thread::sleep_for(chrono::milliseconds{5});
        }));

    poller.join();
}

TEST_F(
    Typed_spsc_ring_buffer_test,
    try_poll_one)
{
    Typed_spsc_ring_buffer<Test_content, 3> ring_buffer;

    EXPECT_FALSE(ring_buffer.try_poll_one([](Test_content content) {}));

    EXPECT_TRUE(ring_buffer.try_push(Test_content()));

    EXPECT_TRUE(ring_buffer.try_poll_one([](Test_content content) {}));
}

TEST_F(
    Typed_spsc_ring_buffer_test,
    poll)
{
    Typed_spsc_ring_buffer<Test_content, 5> ring_buffer;

    EXPECT_EQ(0, ring_buffer.poll([](Test_content content) {}, 100));

    EXPECT_TRUE(ring_buffer.try_push(Test_content()));
    EXPECT_TRUE(ring_buffer.try_push(Test_content()));
    EXPECT_TRUE(ring_buffer.try_push(Test_content()));
    EXPECT_TRUE(ring_buffer.try_push(Test_content()));

    EXPECT_EQ(4, ring_buffer.poll([](Test_content content) {}, 100));
}

TEST_F(
    Typed_spsc_ring_buffer_test,
    poll_until_empty)
{
    Typed_spsc_ring_buffer<Test_content, 5> ring_buffer;

    EXPECT_EQ(0, ring_buffer.poll([](Test_content content) {}, 100));

    EXPECT_TRUE(ring_buffer.try_push(Test_content()));
    EXPECT_TRUE(ring_buffer.try_push(Test_content()));
    EXPECT_TRUE(ring_buffer.try_push(Test_content()));
    EXPECT_TRUE(ring_buffer.try_push(Test_content()));

    EXPECT_EQ(4, ring_buffer.poll_until_empty([](Test_content content) {}));
}




