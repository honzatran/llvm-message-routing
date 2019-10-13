

#include <benchmark/benchmark.h>

#include <routing/message/message.h>
#include <routing/slab_allocator.h>

static void
BM_message_get_int(benchmark::State& state)
{
    auto slab_allocator = std::make_unique<routing::Slab_allocator_t>(
        routing::Malloc_allocator());

    routing::engine::Message message(std::move(slab_allocator));

    for (int i = 0; i < 33; i++)
    {
        message.set_int(i, 1);
    }


    for (auto s : state)
    {
        benchmark::DoNotOptimize(message.get_int(32));
    }
}

BENCHMARK(BM_message_get_int);
