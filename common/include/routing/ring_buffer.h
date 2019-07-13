

#ifndef ROUTING_RING_BUFFER_H
#define ROUTING_RING_BUFFER_H

#include "buffer.h"
#include "stdext.h"

#include <spdlog/fmt/fmt.h>
#include <boost/lockfree/spsc_queue.hpp>

namespace routing
{
template <typename T>
using is_poller = decltype(std::declval<T&>(std::declval<Buffer_view>()));

class SPSC_ring_buffer
{
public:
    SPSC_ring_buffer(
        std::size_t buffer_capacity,
        std::size_t receive_buffer_capacity)
        : m_size(buffer_capacity),
          m_queue(buffer_capacity),
          m_receive_buffer(Buffer(receive_buffer_capacity))
    {
    }

    SPSC_ring_buffer(std::size_t receive_buffer_capacity)
        : m_queue(m_size), m_receive_buffer(Buffer(receive_buffer_capacity))
    {
    }

    SPSC_ring_buffer(SPSC_ring_buffer const& other) = delete;
    SPSC_ring_buffer& operator=(SPSC_ring_buffer const& other) = delete;

    SPSC_ring_buffer(SPSC_ring_buffer&& other)
        : m_size(other.m_size),
          m_queue(other.m_size),
          m_receive_buffer(std::move(other.m_receive_buffer))
    {
    }

    SPSC_ring_buffer& operator=(SPSC_ring_buffer&& other) = delete;

    template <typename T>
    bool push(T const& val)
    {
        std::uint8_t const* dst = reinterpret_cast<std::uint8_t const*>(val);
        return m_queue.push(dst, sizeof(dst));
    }

    std::size_t push(std::uint8_t const* dst, std::size_t size)
    {
        return m_queue.push(dst, size);
    }

    std::size_t push(Buffer_view view)
    {
        return m_queue.push(view.as<std::uint8_t>(), view.get_length());
    }

    template <typename POLLER>
    std::size_t poll(POLLER poller)
    {
        std::size_t prev_position = m_receive_buffer.get_position();

        std::size_t popped = m_queue.pop(
            m_receive_buffer.position_ptr_as<std::uint8_t>(),
            m_receive_buffer.capacity() - prev_position);

        if (popped == 0)
        {
            return 0;
        }

        std::size_t processed
            = poller(m_receive_buffer.slice_from_position(popped));

        m_receive_buffer.reset(processed, prev_position + popped - processed);

        return processed;
    }

private:
    int m_size = 4096;
    boost::lockfree::spsc_queue<std::uint8_t> m_queue;

    Buffer m_receive_buffer;
};

/// This wraper around the boost spsc_queue implements move semantics.
/// Moving instance of this class during live run results in Undefined
/// Behaviour. Also moving does not copy the items inside the moved
/// instance. T must also be default constructible.
template <typename T, std::size_t CAPACITY>
class Typed_spsc_ring_buffer
{
public:
    Typed_spsc_ring_buffer() { m_queue = routing::make_unique<Spsc_queue_t>(); }

    /// Tries to push one item into the spsc queue
    bool try_push(T const& instance) { return m_queue->push(instance); }

    /// Force push one item into the spsc queue, idle strategy
    /// is used to when backpressure is detected
    template <typename IDLE_STRATEGY>
    void force_push(T const& instance, IDLE_STRATEGY idle_strategy)
    {
        while (!m_queue->push(instance))
        {
            idle_strategy();
        }
    }

    /// push one item with number of tries, idle strategy
    /// is used to when backpressure is detected
    template <typename IDLE_STRATEGY>
    bool push(T const& instance, int tries, IDLE_STRATEGY idle_strategy)
    {
        for (int i = 0; i < tries; ++i)
        {
            if (m_queue->push(instance))
            {
                return true;
            }

            idle_strategy();
        }

        return false;
    }

    /// Tries to poll one item from the ring buffer
    template <typename FN>
    bool try_poll_one(FN fn)
    {
        T instance;

        if (m_queue->pop(instance))
        {
            fn(instance);
            return true;
        }

        return false;
    }

    /// Tries to poll maximmally max_items items from the ringbuffer, or
    /// until the ringbuffer is empty
    template <typename FN>
    int poll(FN fn, int max_items)
    {
        T instance;
        int popped_items = 0;

        while (popped_items < max_items && m_queue->pop(instance))
        {
            fn(instance);
            popped_items++;
        }

        return popped_items;
    }

    /// Tries to poll all items from the ringbuffer.
    template <typename FN>
    int poll_until_empty(FN fn)
    {
        T instance;
        int popped_items = 0;

        while (m_queue->pop(instance))
        {
            fn(instance);
            popped_items++;
        }

        return popped_items;
    }

private:
    using Spsc_queue_t
        = boost::lockfree::spsc_queue<T, boost::lockfree::capacity<CAPACITY>>;

    std::unique_ptr<Spsc_queue_t> m_queue;
};

}  // namespace routing

#endif
