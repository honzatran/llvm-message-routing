

#include <benchmark/benchmark.h>

#include <routing/message/message.h>
#include <routing/slab_allocator.h>

static void
BM_message_get_int(benchmark::State& state)
{
    auto slab_allocator = std::make_unique<routing::Slab_allocator_t>(
        routing::Malloc_allocator());

    routing::engine::Message message(std::move(slab_allocator));

    message.set_int(32, 1);

    for (auto _ : state)
    {
        message.get_int(32);
    }
}

BENCHMARK(BM_message_get_int);
