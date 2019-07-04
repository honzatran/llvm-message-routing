
#include <routing/poller_providers.h>
#include <gtest/gtest.h>

using namespace routing;

namespace
{
struct Test_pollable
{
    Poll_result poll() { return Poll_result::stop(); }
};
}


TEST(Round_robin_poller_provider_test, returns_same_pollers)
{
    int const poller_count = 5;

    Basic_thread_factory factory;
    Round_robin_poller_provider<Test_pollable> provider(
        "test", &factory, poller_count, 8, 8);

    std::vector<Poller<Test_pollable>*> pollable_ptrs(poller_count);

    for (std::size_t i = 0; i < poller_count; i++)
    {
        pollable_ptrs[i] = &(provider.get_next_poller());
    }

    for (std::size_t i = 0; i < 2 * poller_count; i++)
    {
        ASSERT_EQ(pollable_ptrs[i % poller_count], &(provider.get_next_poller()));
    }
}

