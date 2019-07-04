
#include <gtest/gtest.h>
#include <routing/poller.h>
#include <routing/latches.h>
#include <routing/stdext.h>
#include <chrono>

using namespace routing;
using namespace std;

namespace
{
class Test_poll
{
public:

    Poll_result poll()
    {
        if (m_function)
        {
            return m_function();
        }

        return Poll_result(1);
    }

    void on_start()
    {
        if (m_on_start_function)
        {
            m_on_start_function();
        }
    }

    void on_stop()
    {
        if (m_on_stop_function)
        {
            m_on_stop_function();
        }
    }

    void set_on_poll(Function_ref<Poll_result()> const& function)
    {
        m_function = function;
    }

    void set_on_start(Function_ref<void()> const& function)
    {
        m_on_start_function = function;
    }

    void set_on_stop(Function_ref<void()> const& function)
    {
        m_on_stop_function = function;
    }

private:

    Function_ref<Poll_result()> m_function;

    Function_ref<void()> m_on_start_function;
    Function_ref<void()> m_on_stop_function;
};
}

class Poller_start_stop_test : public ::testing::Test
{
public:
    Poller<Test_poll> poller;

    class Test_thread_factory : public IThread_factory
    {
    public:
        std::thread create(
            std::string const& name,
            std::function<void()> function) override
        {
            auto latched_functor = [this, function] 
            {
                started++;

                function();

                stopped++;

                m_latch.signal();
            };

            return m_basic_thread_factory.create(name, latched_functor);
        }


        Event_latch m_latch;

        std::atomic_int started{0};
        std::atomic_int stopped{0};

    private:
        Basic_thread_factory m_basic_thread_factory;
    };


protected:
    Test_thread_factory m_thread_factory;

    void SetUp() override
    {
        poller.start(&m_thread_factory);
    }

private:
};

TEST_F(Poller_start_stop_test, after_start_spawn_new_thread)
{
    poller.stop();

    ASSERT_EQ(
        std::cv_status::no_timeout,
        m_thread_factory.m_latch.wait_for(chrono::seconds{1}));

    ASSERT_EQ(1, m_thread_factory.started);
}

TEST_F(Poller_start_stop_test, start_after_start_ignored)
{
    poller.start(&m_thread_factory);

    poller.stop();

    ASSERT_EQ(
        std::cv_status::no_timeout,
        m_thread_factory.m_latch.wait_for(chrono::seconds{1}));

    ASSERT_EQ(1, m_thread_factory.started);
}

TEST_F(Poller_start_stop_test, on_stop_thread_exits)
{
    poller.stop();
    
    ASSERT_EQ(
        std::cv_status::no_timeout,
        m_thread_factory.m_latch.wait_for(chrono::seconds{1}));

    ASSERT_EQ(1, m_thread_factory.stopped);
}

class Poller_test : public ::testing::Test
{
public:
    struct Test_idle_strategy
    {
        int count = 0;

        void operator() (int work_count)
        {
            if (work_count == 0)
            {
                count++;
            }
        }
    };

    Poller<Test_poll, Test_idle_strategy> poller;

private:
    Basic_thread_factory m_thread_factory;

    void SetUp() override
    {
        poller.start(&m_thread_factory);
    }

    void TearDown() override
    {
        poller.stop();
    }
};

TEST_F(Poller_test, after_insert_pollable_polled)
{
    Test_poll test_poll;

    Event_latch polled_latch;

    auto poll = [&polled_latch] ()
    {
        polled_latch.signal();
        return Poll_result(0);
    };
    
    test_poll.set_on_poll(poll);

    poller.insert(test_poll);

    EXPECT_EQ(
        std::cv_status::no_timeout,
        polled_latch.wait_for(chrono::seconds{1}));

    poller.stop();
}

TEST_F(Poller_test, after_insert_on_start_invoked)
{
    Test_poll test_poll;

    Event_latch on_start_latch;

    auto poll = [&on_start_latch] ()
    {
        on_start_latch.signal();
    };
    
    test_poll.set_on_start(poll);

    poller.insert(test_poll);

    EXPECT_EQ(
        std::cv_status::no_timeout,
        on_start_latch.wait_for(chrono::seconds{1}));

    poller.stop();
}

TEST_F(Poller_test, after_pollable_returns_stop_it_is_not_polled)
{
    Test_poll test_poll;

    std::atomic_int counter(0);

    Event_latch latch;

    Event_latch on_stop_latch;

    auto poll = [&counter, &latch] () mutable 
    {
        if (++counter == 5)
        {
            latch.signal();
            return Poll_result::stop();
        }

        return Poll_result::no_work();
    };

    auto on_stop = [&on_stop_latch]
    {
        on_stop_latch.signal();
    };

    test_poll.set_on_poll(poll);
    test_poll.set_on_stop(on_stop);

    poller.insert(test_poll);

    EXPECT_EQ(std::cv_status::no_timeout, latch.wait_for(chrono::seconds{1}));
    EXPECT_EQ(5, counter);

    // on stop invoked
    //
    EXPECT_EQ(
        std::cv_status::no_timeout, on_stop_latch.wait_for(chrono::seconds{1}));

    poller.stop();
}

TEST_F(Poller_test, after_multiple_pollable_returns_stop_it_is_not_polled)
{
    Test_poll test_poll1;
    Test_poll test_poll2;
    Test_poll test_poll3;

    std::atomic_int counter1(0);
    std::atomic_int counter2(0);
    std::atomic_int counter3(0);

    Latch latch(20);

    auto poll_factory = [] (std::atomic_int& counter, Latch& latch, int count)
    {
        return [&counter, &latch, count] () mutable 
        {

            if (++counter == count)
            {
                latch.count_down();
                return Poll_result::stop();
            }

            latch.count_down();

            return Poll_result::no_work();
        };
    };

    auto poll1 = poll_factory(counter1, latch, 5);
    auto poll2 = poll_factory(counter2, latch, 10);
    auto poll3 = poll_factory(counter3, latch, 5);

    test_poll1.set_on_poll(poll1);
    test_poll2.set_on_poll(poll2);
    test_poll3.set_on_poll(poll3);

    poller.insert(test_poll1);
    poller.insert(test_poll2);
    poller.insert(test_poll3);

    EXPECT_EQ(std::cv_status::no_timeout, latch.wait_for(chrono::seconds{1})) 
        << latch.get_count();

    EXPECT_EQ(5, counter1);
    EXPECT_EQ(10, counter2);
    EXPECT_EQ(5, counter3);

    poller.stop();
}

TEST_F(Poller_test, on_nothing_polled_idle_strategy_is_invoked)
{
    Test_poll test_poll;

    std::atomic_int counter(0);
    Event_latch latch;

    auto poll = [&counter, &latch] () mutable 
    {
        if (++counter == 5)
        {
            latch.signal();
            return Poll_result::stop();
        }

        return Poll_result::no_work();
    };

    test_poll.set_on_poll(poll);

    poller.insert(test_poll);

    EXPECT_EQ(std::cv_status::no_timeout, latch.wait_for(chrono::seconds{1}));

    poller.stop();

    EXPECT_LE(5, poller.get_idle_strategy().count);
}

TEST_F(Poller_test, when_poller_stopped_registered_pollable_on_stop_invoked)
{
    Test_poll test_poll;

    Event_latch on_start_latch, on_stop_latch;

    auto on_start = [&on_start_latch] () mutable 
    {
        on_start_latch.signal();
    };

    auto on_stop = [&on_stop_latch] () mutable 
    {
        on_stop_latch.signal();
    };

    test_poll.set_on_start(on_start);
    test_poll.set_on_stop(on_stop);

    poller.insert(test_poll);

    EXPECT_EQ(
        std::cv_status::no_timeout,
        on_start_latch.wait_for(chrono::seconds{1}));

    EXPECT_EQ(
        std::cv_status::timeout,
        on_stop_latch.wait_for(chrono::milliseconds{100}));

    poller.stop();

    EXPECT_EQ(
        std::cv_status::no_timeout,
        on_stop_latch.wait_for(chrono::seconds{1}));
}

