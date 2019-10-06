

#ifndef ROUTING_ENGINE_MESSAGE_H
#define ROUTING_ENGINE_MESSAGE_H

#include <absl/container/internal/container_memory.h>
#include <absl/container/internal/layout.h>
#include <absl/container/internal/raw_hash_set.h>
#include <absl/time/time.h>

#include "absl/base/internal/bits.h"
#include "message_key.h"

#include <routing/buffer.h>
#include <routing/decimal.h>
#include <routing/fmt.h>
#include <routing/slab_allocator.h>

#include <absl/strings/string_view.h>
#include <absl/time/time.h>
#include <absl/types/span.h>
#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>

namespace routing::engine
{
namespace detail
{
/// Constants for computing the city hash for integer types
class City_hash
{
public:
    static constexpr uint64_t kMul = sizeof(size_t) == 4
                                         ? uint64_t{0xcc9e2d51}
                                         : uint64_t{0x9ddfea08eb382d69};

    static std::uint32_t compute(std::int32_t key)
    {
        key *= kMul;
        return (key ^ (key >> 16));
    }
};

/// View into the basic block with the message with the message
class Message_view
{
public:
    Message_view(Buffer_view view) {}

    std::uint8_t* get_key_location(
        std::int32_t key,
        engine::Field_type field_type)
    {
        std::uint32_t const index = City_hash::compute(key);
    }

    std::size_t capacity() const { return m_view.as_value<std::int32_t>(0); }

private:
    Buffer_view m_view;
};

inline std::size_t
next_power_of_2(std::size_t value)
{
    return (sizeof(size_t) == 8)
               ? 1
                     << ((sizeof(size_t) * 8)
                         - absl::base_internal::CountLeadingZeros64(value - 1))
               : 1
                     << ((sizeof(size_t) * 8)
                         - absl::base_internal::CountLeadingZeros32(value - 1));
}

inline bool
is_power_of2(std::size_t value)
{
    return (value & (value - 1)) == 0;
}

/// Returns the initial capacity of the message. Using the next power of 2 from
/// the capacity.
///
/// \return the initial capacity of the message.
inline std::size_t
initial_capacity(std::size_t capacity)
{
    return std::max(16UL, next_power_of_2(capacity));
}

/// Returns how many elements can be added to the message, until a regrowth
/// and rehash is needed.
///
/// \param the message capacity
///
/// \return the initial capacity of the message.
inline std::size_t
compute_growth_left(std::size_t capacity)
{
    return capacity - (capacity / 8);
}

/// Returns a hash seed.
///
/// The seed consists of the ctrl_ pointer, which adds enough entropy to ensure
/// non-determinism of iteration order in most cases.
inline size_t
hash_seed(const Message_chunk* key_part)
{
    // The low bits of the pointer have little or no entropy because of
    // alignment. We shift the pointer to try to use higher entropy bits. A
    // good number seems to be 12 bits, because that aligns with page size.
    return reinterpret_cast<uintptr_t>(key_part) >> 12;
}

/// returns the h1 hash used to address the chunks
inline size_t
H1(size_t hash, const Message_chunk* key_part)
{
    return (hash >> 7) ^ hash_seed(key_part);
}

/// returns the h2 part of hash used to address the slots within chunks
inline absl::container_internal::ctrl_t
H2(size_t hash)
{
    return hash & 0x7F;
}

class quadratic_probe
{
public:
    quadratic_probe(size_t hash, size_t mask)
    {
        m_mask   = mask;
        m_offset = hash & m_mask;
    }

    size_t offset() const { return m_offset; }
    size_t offset(size_t i) const { return (m_offset + i) & m_mask; }

    void next()
    {
        ++m_index;
        m_offset += m_index;
        m_offset &= m_mask;
    }
    // 0-based probe index. The i-th probe in the probe sequence.
    size_t index() const { return m_index; }

private:
    size_t m_mask;
    size_t m_offset;
    size_t m_index = 0;
};

}  // namespace detail

class Message
{
public:
    Message() = default;
    /// Expects an underlying serialized message
    /// @param memory pointer towards the memory
    Message(
        std::unique_ptr<Slab_allocator_t> slab_allocator,
        std::size_t capacity           = 31,
        std::size_t expected_mem_usage = 0);

    void set_int(std::int32_t key, std::int32_t value)
    {
        insert<Field_type::INT>(key, value);
    }

    void set_long(std::int32_t key, std::int64_t value)
    {
        insert<Field_type::LONG>(key, value);
    }

    void set_double(std::int32_t key, double value)
    {
        insert<Field_type::DOUBLE>(key, value);
    }

    void set_decimal(std::int32_t key, routing::Decimal value)
    {
        insert<Field_type::DECIMAL>(key, value);
    }

    void set_time(std::int32_t key, absl::Duration day_time);

    void set_date(std::int32_t key, absl::Time time);

    void set_string(std::int32_t key, absl::string_view value);

    void set_custom_data(std::int32_t key, absl::Span<std::uint8_t> data);

    void set_message(std::int32_t key, Message const& message);


    bool has_int(std::int32_t key) { return find<Field_type::INT>(key); }

    bool has_long(std::int32_t key) { return find<Field_type::LONG>(key); }

    bool has_double(std::int32_t key) { return find<Field_type::DOUBLE>(key); }

    bool has_decimal(std::int32_t key)
    {
        return find<Field_type::DECIMAL>(key);
    }

    void has_time(std::int32_t key);

    void has_date(std::int32_t key);

    void has_string(std::int32_t key);

    void has_custom_data(std::int32_t key);

    void has_message(std::int32_t key);

    std::int32_t get_int(std::int32_t key)
    {
        auto value_view = find<Field_type::INT>(key);
        return value_view.as<Field_type::INT>();
    }

    std::int64_t get_long(std::int32_t key)
    {
        auto value_view = find<Field_type::LONG>(key);
        return value_view.as<Field_type::LONG>();
    }

    double get_double(std::int32_t key)
    {
        auto value_view = find<Field_type::DOUBLE>(key);
        return value_view.as<Field_type::DOUBLE>();
    }

    Decimal get_decimal(std::int32_t key)
    {
        auto value_view = find<Field_type::DECIMAL>(key);
        return value_view.as<Field_type::DECIMAL>();
    }

    absl::Duration get_day_time(std::int32_t key);

    absl::Time get_time(std::int32_t key);

    absl::string_view get_string(std::int32_t key);

    absl::Span<std::uint8_t> get_custom_data(std::int32_t key);

    Message get_message(std::int32_t key);

    /// Returns the actual size of this map
    std::size_t size() const { return m_size; }

    /// Returns the current capacity of this map
    std::size_t capacity() const { return m_capacity; }

    /// Returns whether this map is empty
    bool empty() const { return !size(); }

private:
    using Message_layout_t = absl::container_internal::
        Layout<Message_chunk, absl::container_internal::Ctrl>;

    /// Holds the slab allocator towards this message,
    /// each message has it's own slab allocator
    std::unique_ptr<Slab_allocator_t> m_slab_allocator;

    /// Holds the capacity, i.e. how much elements can the map holds
    std::size_t m_capacity;

    /// Holds the actual number of elements within the map
    std::size_t m_size = 0;

    /// Holds the number of chunks within a single message
    std::size_t m_chunks_count;

    /// Holds the count of how much new keys can be inserted until we rehash.
    std::size_t m_growth_left;

    /// Pointer towards the key parts
    Message_chunk* m_chunks;

    /// Create a layout of the message
    ///
    /// \param chunk_count number of chunks within a single message
    /// \param sentinel_count number of sentinel ctrl bytes at the end of
    ///        message
    Message_layout_t create_layout(
        std::size_t chunk_count,
        std::size_t sentinel_count)
    {
        return Message_layout_t(chunk_count, sentinel_count);
    }

    void reset_ctrl()
    {
        for (int i = 0; i < m_chunks_count; i++)
        {
            m_chunks[i].reset();
        }

        absl::container_internal::ctrl_t* sentinel
            = reinterpret_cast<absl::container_internal::ctrl_t*>(
                &m_chunks[m_chunks_count]);

        sentinel[0] = absl::container_internal::kSentinel;

        for (int i = 1; i < 16; i++)
        {
            sentinel[i] = absl::container_internal::kSentinel;
        }
    }

    detail::quadratic_probe create_probe(std::size_t hash) const
    {
        return detail::quadratic_probe(
            detail::H1(hash, m_chunks), m_chunks_count - 1);
    }

    /// Inserts the element into a hashmap
    /// Finds an empty slot within the hashmap and sets that slot to new element.
    template <Field_type FIELD_TYPE>
    void insert(
        std::int32_t key,
        typename Field_type_policy<FIELD_TYPE>::cpp_type value)
    {
        auto value_view = find_or_prepare_insert<FIELD_TYPE>(key);

        if (!value_view.second)
        {
            m_size++;
            m_growth_left--;
        }

        value_view.first.template set<FIELD_TYPE>(key, value);
    }

    /// Finds if a key exists within the message
    ///
    /// \param key element key
    /// \tparam field_type type of the element associated with the key
    template <Field_type field_type>
    Message_value_view find(std::int32_t key) const
    {
        std::size_t hash = detail::City_hash::compute(key);
        auto probe       = create_probe(hash);

        auto h2_value = detail::H2(hash);

        for (int tries = 0; tries < m_chunks_count; ++tries)
        {
            Message_chunk* chunk = &m_chunks[probe.offset()];

            int mask = chunk->get_match_position<field_type>(h2_value);

            if (mask != 0)
            {
                auto value = chunk->match_key(key, mask);

                if (value)
                {
                    return value;
                }
            }

            if (chunk->match_empty())
            {
                return Message_value_view();
            }

            probe.next();
        }

        return Message_value_view();
    }

    /// Returns a pair with a view where the first of the pair
    /// is a view where the element could be inserted, the second
    /// boolean parameter returns whether this key was already associated
    /// with a value
    ///
    /// \param key the key of the field
    /// \tparam field_type type of the key
    /// \return the pair where first elements allows the insertion, and the
    ///         second is true whether this is a new key within the hashmap
    template <Field_type field_type>
    std::pair<Message_value_view, bool> find_or_prepare_insert(std::int32_t key)
    {
        std::size_t hash = detail::City_hash::compute(key);
        auto probe       = create_probe(hash);

        auto h2_value = detail::H2(hash);

        for (int tries = 0; tries < m_chunks_count; ++tries)
        {
            Message_chunk* chunk = &m_chunks[probe.offset()];

            int mask = chunk->get_match_position<field_type>(h2_value);

            if (mask != 0)
            {
                auto value_view = chunk->match_key(key, mask);

                if (value_view)
                {
                    return {chunk->match_key(key, mask), true};
                }
            }

            if (chunk->match_empty())
            {
                break;
            }

            probe.next();
        }

        return {prepare_insert<field_type>(hash, h2_value), false};
    }

    /// Finds first available space for the hash within the parameter
    /// If the hash map is full, the message is resized and rehashed and new
    /// available position is find.
    ///
    /// \param hash     the hash we are looking for position
    /// \param h2_value the h2 value of the hash = H2(hash)
    ///
    /// \return instance of Message_value_view pointing to an available slot
    template <Field_type field_type>
    Message_value_view prepare_insert(
        std::size_t hash,
        absl::container_internal::ctrl_t h2_value)
    {
        auto result = find_first_non_null(hash, field_type);

        if (!result || (m_growth_left == 0 && !result.is_deleted()))
        {
            // rehash the hash map
            rehash_and_grow_if_necessary();

            result = find_first_non_null(hash, field_type);
        }

        result.set_ctrl(h2_value);
        return result;
    }

    Message_value_view find_first_non_null(
        std::size_t hash,
        Field_type field_type)
    {
        auto probe = create_probe(hash);

        for (int tries = 0; tries < m_chunks_count; ++tries)
        {
            Message_chunk* chunk = &m_chunks[probe.offset()];

            auto position = chunk->first_available_position();

            if (position)
            {
                return position;
            }

            assert(probe.index() < m_chunks_count && "full table!");
            probe.next();
        }

        return Message_value_view();
    }

    /// Regrows and rehashes the hashmap if it's needed.
    void rehash_and_grow_if_necessary()
    {
        if (size() <= detail::compute_growth_left(capacity()) / 2)
        {
            drop_deletes_without_resize();
        }
        else
        {
            resize(2 * m_capacity);
        }
    }

    void drop_deletes_without_resize() {}

    /// Resize the message and rehashes the inserted items
    void resize(std::size_t capacity)
    {
        assert(detail::is_power_of2(capacity) && "capacity must be power of 2");

        Message_chunk* old_chunks          = m_chunks;
        std::size_t const old_capacity     = m_capacity;
        std::size_t const old_chunks_count = m_chunks_count;
        m_capacity                         = capacity;

        initialize_chunks();

        for (std::size_t i = 0; i < old_chunks_count; ++i)
        {
            Message_chunk* old_chunk = &old_chunks[i];

            for (int index : old_chunk->match_used())
            {
                auto value = old_chunk->get_value_view(index);

                std::size_t new_hash  = detail::City_hash::compute(value.key());
                Field_type field_type = value.type();

                auto new_position = find_first_non_null(new_hash, field_type);

                new_position.set_ctrl(detail::H2(new_hash));
                new_position.set(field_type, value.key(), value.get());
            }
        }
    }

    /// Allocates new memory  and setups the fields dependent on the m_capacity
    void initialize_chunks()
    {
        m_chunks_count = m_capacity / Message_chunk::k_width;
        m_growth_left  = detail::compute_growth_left(m_capacity);

        auto layout = create_layout(m_chunks_count, Message_chunk::k_width);

        std::uint8_t* data
            = reinterpret_cast<std::uint8_t*>(m_slab_allocator->allocate(
                layout.AllocSize(), Message_chunk::k_width));

        m_chunks = layout.Pointer<0>(data);

        absl::container_internal::SanitizerPoisonMemoryRegion(
            data, layout.AllocSize());

        reset_ctrl();
    }
};
}  // namespace routing::engine

#endif
