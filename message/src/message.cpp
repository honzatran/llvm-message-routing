
#include <immintrin.h>
#include <routing/message/message.h>
#include "absl/container/internal/container_memory.h"
#include "absl/container/internal/raw_hash_set.h"

#include <routing/fmt.h>

using namespace routing::engine;

routing::engine::Message::Message(
    std::unique_ptr<Slab_allocator_t> slab_allocator,
    std::size_t capacity,
    std::size_t expected_mem_usage)
    : m_slab_allocator(std::move(slab_allocator)),
      m_capacity(detail::initial_capacity(capacity))
{
    initialize_chunks();
}
