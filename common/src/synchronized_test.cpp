

#include <routing/synchronized.h>

#include <gtest/gtest.h>
#include <thread>

using namespace routing;
using namespace std;


TEST(Synchronized_test, single_thread)
{
    Synchronized<std::vector<int>> synchronized_vector;

    auto lock = synchronized_vector.lock();

    lock->push_back(1);

    EXPECT_EQ(1, lock->size());

    std::vector<int> copy = *lock;

    EXPECT_EQ(1, copy.size());

    lock->push_back(1);

    EXPECT_EQ(1, copy.size());
    EXPECT_EQ(2, lock->size());
}

TEST(Synchronized_test, single_thread_copy)
{
    Synchronized<std::vector<int>> synchronized_vector;

    synchronized_vector = { 1, 2, 3 };

    std::vector<int> copy = synchronized_vector.copy();

    EXPECT_EQ(3, copy.size());

    auto locked_vector = synchronized_vector.lock();

    locked_vector->push_back(1);

    EXPECT_EQ(3, copy.size());
    EXPECT_EQ(4, locked_vector->size());
}

TEST(Synchronized_test, multi_thread_inc)
{
    Synchronized<int> sync_int(0);

    std::thread tmp = std::thread([&sync_int] {
        for (int i = 0; i < 1000; ++i)
        {
            auto locked = sync_int.lock();

            *locked += 1;
        }
    });

    for (int i = 0; i < 1000; ++i)
    {
        auto locked = sync_int.lock();

        *locked += 1;
    }

    tmp.join();

    int count = sync_int.copy();

    EXPECT_EQ(2000, count);
}

TEST(Synchronized_test, multi_thread_assignment)
{
    Synchronized<int> sync_int1(0);
    Synchronized<int> sync_int2(0);

    std::thread tmp = std::thread([&sync_int1, &sync_int2] {
        for (int i = 0; i < 1000; ++i)
        {
            sync_int1 = sync_int2;
        }
    });

    for (int i = 0; i < 1000; ++i)
    {
        sync_int2 = sync_int1;
    }

    tmp.join();
}
