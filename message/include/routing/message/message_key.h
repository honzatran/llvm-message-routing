
#ifndef ROUTING_ENGINE_MESSAGE_KEY_H
#define ROUTING_ENGINE_MESSAGE_KEY_H

#include <emmintrin.h>
#include <immintrin.h>
#include <cstdint>
#include <iterator>
#include <type_traits>

#include <iostream>

#include <absl/container/internal/raw_hash_set.h>
#include <absl/meta/type_traits.h>
#include "have_intrinsics.h"

namespace routing::engine
{
namespace detail
{
/// Returns the number of bytes for each key
constexpr std::int32_t
key_width()
{
    return 28;
}

/// Returns the mask for the key
constexpr std::int32_t
key_mask()
{
    return (1 << key_width()) - 1;
}

}  // namespace detail

/// Represents the allowed types of field within message.
enum class Field_type : std::uint8_t
{
    INT           = 0,
    LONG          = 1,
    DOUBLE        = 2,
    DECIMAL       = 3,
    TIME          = 4,
    DATE          = 5,
    STRING        = 6,
    CUSTOM_DATA   = 7,
    MESSAGE_ARRAY = 8
};

template <Field_type FIELD_TYPE>
struct Field_type_policy
{
    /// corresponding cpp type
    // using cpp_type = std::int32_t;
    //
    /// conversion from the raw std::int64_t raw_value back to the actual type
    // static cpp_type convert_back(std::int64_t* raw_value) { return *raw_value; }
    //
    //
    /// conversion from actual type to the raw std::int64_t which is stored inside the hashmap.
    // static std::int64_t conver_to(cpp_type value) { return value; }
};

template <>
struct Field_type_policy<Field_type::INT>
{
    using cpp_type = std::int32_t;

    static cpp_type convert_back(std::int64_t* raw_value) { return *raw_value; }

    static std::int64_t convert_to(cpp_type value) { return value; }
};

template <>
struct Field_type_policy<Field_type::LONG>
{
    using cpp_type = std::int64_t;

    static cpp_type convert_back(std::int64_t* raw_value) { return *raw_value; }

    static std::int64_t convert_to(cpp_type value) { return value; }
};

template <>
struct Field_type_policy<Field_type::DOUBLE>
{
    using cpp_type = double;

    static cpp_type convert_back(std::int64_t* raw_value)
    {
        return static_cast<double>(*raw_value);
    }

    static std::int64_t convert_to(cpp_type value)
    {
        return static_cast<std::int64_t>(value);
    }
};

class Message_key
{
    static constexpr std::int32_t max() { return (1 << detail::key_width()); }

    explicit Message_key(std::int32_t key) : m_key(key) {}

    /// Gets the integer representing the actual key.
    std::int32_t key() const { return m_key; }

private:
    std::int32_t m_key;
};

/// Represents a combined key inside the routing message with the
/// field type
class Combined_message_key
{
public:
    explicit Combined_message_key(std::int32_t key, Field_type type)
    {
        m_key_value = key
                      | (std::underlying_type<Field_type>::type(type)
                         << detail::key_width());
    }

    /// Gets the integer representing the actual key.
    std::int32_t key() const { return m_key_value & detail::key_mask(); }

    /// Gets the field type with this key.
    Field_type field_type() const
    {
        int raw_type = m_key_value >> detail::key_width();
        return Field_type(raw_type & 0xF);
    }

private:
    std::int32_t m_key_value;
};

#if RA_HAVE_SSE2

struct Message_chunk_sse_properties
{
    static constexpr std::size_t k_width = 16;

    using BitMask = absl::container_internal::BitMask<std::uint32_t, k_width>;
};

class Message_chunk_sse_group
{
public:
    using BitMask = Message_chunk_sse_properties::BitMask;

    Message_chunk_sse_group(absl::container_internal::ctrl_t* ptr)
    {
        m_ctrl = _mm_loadu_si128(reinterpret_cast<const __m128i*>(ptr));
    };

    int match(absl::container_internal::ctrl_t h2_hash) const
    {
        auto match = _mm_set1_epi8(h2_hash);
        return _mm_movemask_epi8(_mm_cmpeq_epi8(match, m_ctrl));
    }

    int match_empty() const
    {
#if RA_HAVE_SSE3
        return _mm_movemask_epi8(_mm_sign_epi8(m_ctrl, m_ctrl));

#else
        return match(absl::container_internal::Ctrl::kEmpty);
#endif
    }

    BitMask match_empty_or_deleted() const
    {
        auto special = _mm_set1_epi8(absl::container_internal::kSentinel);

        return BitMask(_mm_movemask_epi8(
            absl::container_internal::_mm_cmpgt_epi8_fixed(special, m_ctrl)));
    }

    BitMask match_used() const
    {
        auto sentinel = _mm_set1_epi8(absl::container_internal::kSentinel);

        return BitMask(_mm_movemask_epi8(_mm_cmpgt_epi8(m_ctrl, sentinel)));
    }

private:
    __m128i m_ctrl;
};

/// SSE based implementation of the group
class Message_value_group_sse_types
{
public:
    Message_value_group_sse_types() { m_types = _mm_set1_epi8(-1); }

    Message_value_group_sse_types(__m128i types) : m_types(types) {}

    /// Gets the mask, with bit set where the type in the underlying
    /// type is the same like parameter field_type
    ///
    /// \param field_type type of the field
    template <Field_type field_type>
    std::int32_t get_mask() const
    {
        __m128i type_mask
            = _mm_set1_epi8(static_cast<std::uint8_t>(field_type));

        __m128i compare_mask = _mm_cmpeq_epi8(type_mask, m_types);

        return _mm_movemask_epi8(compare_mask);
    }

    std::int32_t get_mask(Field_type field_type) const
    {
        __m128i type_mask
            = _mm_set1_epi8(static_cast<std::uint8_t>(field_type));

        __m128i compare_mask = _mm_cmpeq_epi8(type_mask, m_types);

        return _mm_movemask_epi8(compare_mask);
    }

    /// sets the type byte in the mask to the specific type
    /// index must be < 16
    void set_type(std::uint8_t index, Field_type field_type)
    {
        std::uint8_t* value = reinterpret_cast<std::uint8_t*>(&m_types);
        value[index]        = static_cast<std::uint8_t>(field_type);
    }

    Field_type operator[](int index) const
    {
        auto field_types = reinterpret_cast<Field_type const*>(&m_types);
        return field_types[index];
    }

    /// Returns whether the type is in the mask.
    ///
    /// \param type of the field
    bool contains(Field_type field_type) const
    {
        return get_mask(field_type) > 0;
    }

private:
    __m128i m_types;
};

using Message_chunk_properties  = Message_chunk_sse_properties;
using Message_chunk_group       = Message_chunk_sse_group;
using Message_value_group_types = Message_value_group_sse_types;
#else

class Message_chunk_properties_portability;
class Message_value_group_portability;

using Mesage_chunk_properties   = Message_chunk_properties_portability;
using Message_value_group_types = Message_value_group_portability;
#endif

#if RA_HAVE_SSE2

class Message_value_group_sse_keys
{
public:
    Message_value_group_sse_keys()
    {
        m_keys[0] = _mm_setzero_si128();
        m_keys[1] = _mm_setzero_si128();
        m_keys[2] = _mm_setzero_si128();
        m_keys[3] = _mm_setzero_si128();
    }

    bool all_zeros()
    {
        for (int i = 0; i < 4; ++i)
        {
            __m128i cmp_result = _mm_cmpeq_epi8(m_keys[i], _mm_setzero_si128());

            if (_mm_movemask_epi8(cmp_result) != 0xFFFF)
            {
                return false;
            }
        }

        return true;
    }

    void set_key(std::uint8_t index, std::int32_t key)
    {
        std::uint8_t key_field_index        = index / 4;
        std::uint8_t index_within_key_field = index % 4;

        std::int32_t* sse_key_field
            = reinterpret_cast<std::int32_t*>(&m_keys[key_field_index]);

        sse_key_field[index_within_key_field] = key;
    }

    int contains(std::int32_t key) const
    {
        __m128i key_mask = _mm_set1_epi32(key);

        for (int i = 0; i < 4; ++i)
        {
            __m128i cmp_result = _mm_cmpeq_epi32(m_keys[i], key_mask);

            std::int32_t value = _mm_movemask_epi8(cmp_result);

            std::int32_t mask   = 1 | (1 << 4) | (1 << 8) | (1 << 12);
            std::int32_t filter = value & mask;

            if (filter != 0)
            {
                return i * 4 + __builtin_ctz(filter) / 4;
            }
        }

        return -1;
    }

    int contains(std::int32_t key, std::int32_t mask) const
    {
        __m128i key_mask = _mm_set1_epi32(key);

        for (int i = 0; i < 4; ++i)
        {
            std::int32_t key_mask_position = (mask >> (4 * i)) & 0xF;

            if (key_mask_position != 0)
            {
                __m128i cmp_result = _mm_cmpeq_epi32(m_keys[i], key_mask);

                std::int32_t value = _mm_movemask_epi8(cmp_result);

                std::int32_t mask   = 1 | (1 << 4) | (1 << 8) | (1 << 12);
                std::int32_t filter = value & mask;

                if (filter != 0)
                {
                    return i * 4 + __builtin_ctz(filter) / 4;
                }
            }
        }

        return -1;
    }

    int operator[](int index) const
    {
        return reinterpret_cast<std::int32_t const*>(&m_keys[0])[index];
    }

private:
    __m128i m_keys[4];
};

using Message_value_group_keys = Message_value_group_sse_keys;
#else

#endif

/// Allows accesses and insertion elements to the map
class Message_value_view;

/// Message value group is stored inside a message and has following layout:
/// 16 byte for h2 of keys
/// 16 byte for type
/// 16 int32 for keys
/// 16 int64 for values or pointers
class Message_chunk
{
public:
    /// Number of slots per chunk
    static constexpr std::size_t k_width = Message_chunk_properties::k_width;

    using Bitmask = Message_chunk_properties::BitMask;

    /// Get all positions matching the field type and the ctrl in this group
    template <Field_type field_type>
    int get_match_position(absl::container_internal::ctrl_t ctrl)
    {
        Message_chunk_group group{&m_ctrl[0]};

        int key_mask = group.match(ctrl);

        if (key_mask)
        {
            int type_mask = m_types.get_mask<field_type>();

            return key_mask & type_mask;
        }

        return 0;
    }

    bool match_empty()
    {
        Message_chunk_group group{&m_ctrl[0]};

        return group.match_empty() != 0;
    }

    Message_value_view const match_key(std::int32_t key, int mask) const;

    Message_value_view match_key(std::int32_t key, int mask);

    Message_value_view first_available_position();

    Message_value_view const get_value_view(std::size_t index) const;
    Message_value_view get_value_view(std::size_t index);

    bool is_sentinel() const
    {
        return m_ctrl[0] == absl::container_internal::kSentinel;
    }

    void reset()
    {
        for (int i = 0; i < k_width; i++)
        {
            m_ctrl[i] = absl::container_internal::kEmpty;
        }
    }

    Bitmask match_used() const
    {
        Message_chunk_group group{
            const_cast<absl::container_internal::ctrl_t*>(&m_ctrl[0])};

        return group.match_used();
    }

    std::int32_t key(int index) { return m_keys[index]; }

private:
    friend class Message_value_view;

    absl::container_internal::ctrl_t m_ctrl[k_width];
    Message_value_group_types m_types;
    Message_value_group_keys m_keys;
    std::int64_t m_value[k_width];
};

/// Allows accesses and insertion elements to the map
class Message_value_view
{
public:
    Message_value_view() = default;

    Message_value_view(Message_chunk const* chunk, std::uint8_t position)
        : m_group(const_cast<Message_chunk*>(chunk)), m_position(position)
    {
    }

    Message_value_view(Message_chunk* chunk, std::uint8_t position)
        : m_group(chunk), m_position(position)
    {
    }

    operator std::int64_t*() const { return &(m_group->m_value[m_position]); }

    operator bool() const { return m_group; }

    /// Sets the message value view to the type and value, this method is used
    /// for copying values, when resizing the hashmap.As it accepts the value as
    /// int64.
    void set(Field_type field_type, std::int32_t key, std::int64_t value)
    {
        m_group->m_types.set_type(m_position, field_type);
        m_group->m_keys.set_key(m_position, key);
        m_group->m_value[m_position] = value;
    }

    /// Sets the message value view to the type and value, this method is for
    /// for directly setting the value, when inserting new elements to the map.
    /// Hence it's templated
    template <Field_type FIELD_TYPE>
    void set(
        std::int32_t key,
        typename Field_type_policy<FIELD_TYPE>::cpp_type value)
    {
        m_group->m_types.set_type(m_position, FIELD_TYPE);
        m_group->m_keys.set_key(m_position, key);
        m_group->m_value[m_position]
            = Field_type_policy<FIELD_TYPE>::convert_to(value);
    }

    void set_ctrl(absl::container_internal::ctrl_t h2)
    {
        m_group->m_ctrl[m_position] = h2;
    }

    Message_value_view& wrap(Message_chunk* chunk, std::uint8_t position)
    {
        m_group    = chunk;
        m_position = position;

        return *this;
    }

    bool is_deleted() const
    {
        return m_group->m_ctrl[m_position]
               == absl::container_internal::kDeleted;
    }

    bool is_empty() const
    {
        return m_group->m_ctrl[m_position] == absl::container_internal::kEmpty;
    }

    Field_type type() const { return m_group->m_types[m_position]; }

    std::int32_t key() const { return m_group->m_keys[m_position]; }

    std::int64_t get() const { return m_group->m_value[m_position]; }

    template <Field_type FIELD_TYPE>
    auto as() const -> typename Field_type_policy<FIELD_TYPE>::cpp_type
    {
        return Field_type_policy<FIELD_TYPE>::convert_back(
            &m_group->m_value[m_position]);
    }

private:
    Message_chunk* m_group;
    std::uint8_t m_position;
};

inline Message_value_view
Message_chunk::first_available_position()
{
    Message_chunk_group group{&m_ctrl[0]};

    auto mask = group.match_empty_or_deleted();

    if (mask)
    {
        return Message_value_view(this, mask.LowestBitSet());
    }

    return Message_value_view();
}

inline Message_value_view const
Message_chunk::match_key(std::int32_t key, int mask) const
{
    int position = m_keys.contains(key, mask);

    if (position < 0)
    {
        return Message_value_view();
    }

    return Message_value_view(this, position);
}

inline Message_value_view
Message_chunk::match_key(std::int32_t key, int mask)
{
    int position = m_keys.contains(key, mask);

    if (position < 0)
    {
        return Message_value_view();
    }

    return Message_value_view(this, position);
}

inline Message_value_view const
Message_chunk::get_value_view(std::size_t index) const
{
    return Message_value_view(this, index);
}

inline Message_value_view
Message_chunk::get_value_view(std::size_t index)
{
    return Message_value_view(this, index);
}

}  // namespace routing::engine

#endif
