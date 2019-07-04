

#ifndef ROUTING_BUFFER_H
#define ROUTING_BUFFER_H

#include <cstdint>
#include <cstring>
#include <vector>
#include <type_traits>
#include <routing/stdext.h>

namespace routing
{
class Buffer_view
{
public:
    using value_type             = std::uint8_t;
    using pointer                = std::uint8_t*;
    using const_pointer          = std::uint8_t const*;
    using reference              = std::uint8_t&;
    using const_reference        = std::uint8_t const&;
    using const_iterator         = std::uint8_t const*;
    using iterator               = const_iterator;
    using const_reverse_iterator = std::reverse_iterator<iterator>;
    using reverse_iterator       = const_reverse_iterator;
    using size_type              = std::size_t;
    using difference_type        = std::ptrdiff_t;

    Buffer_view()  = default;

    Buffer_view(std::uint8_t* data, std::size_t length)
        : m_data(data), m_length(length)
    {
    }

    Buffer_view(void* data, std::size_t length)
        : m_data(reinterpret_cast<std::uint8_t*>(data)), m_length(length)
    {
    }

    template <typename T>
    Buffer_view(T* data, std::size_t length)
        : Buffer_view(reinterpret_cast<std::uint8_t*>(data), sizeof(T) * length)
    {
    }

    Buffer_view(Buffer_view const& other) = default;
    Buffer_view& operator=(Buffer_view const& other) = default;
    Buffer_view(Buffer_view&& other)                 = default;
    Buffer_view& operator=(Buffer_view&& other) = default;

    std::size_t get_length() const { return m_length; }

    template <typename T>
    T const* as() const
    {
        return reinterpret_cast<T*>(m_data);
    }

    template <typename T>
    T const* as(std::size_t offset) const
    {
        return reinterpret_cast<T*>(m_data + offset);
    }

    template <typename T>
    T as_value(std::size_t offset) const
    {
        return *(reinterpret_cast<T*>(&m_data[offset]));
    }

    Buffer_view slice(std::size_t offset, std::size_t length) const
    {
        return Buffer_view{&m_data[offset], length};
    }

    template <typename CONSUMER, typename SENTINEL>
    std::size_t accept(CONSUMER consumer, SENTINEL sentinel)
    {
        std::size_t start = 0;

        while (start < m_length)
        {
            int shift_length = sentinel(slice(start, m_length - start));

            if (shift_length < 0)
            {
                break;
            }

            consumer(slice(start, shift_length));

            start += shift_length;
        }

        return start;
    }

    // iterators
    const_iterator begin() const noexcept { return m_data; }
    const_iterator end() const noexcept { return m_data + m_length; }
    const_iterator cbegin() const noexcept { return begin(); }
    const_iterator cend() const noexcept { return end(); }
    const_reverse_iterator rbegin() const noexcept
    {
        return const_reverse_iterator(end());
    }

    const_reverse_iterator rend() const noexcept
    {
        return const_reverse_iterator(begin());
    }

    const_reverse_iterator crbegin() const noexcept { return rbegin(); }
    const_reverse_iterator crend() const noexcept { return rend(); }

    std::uint8_t const& operator[] (std::size_t offset) const noexcept
    {
        return m_data[offset];
    }

private:
    friend class Mut_buffer_view;


    std::uint8_t* m_data{nullptr};
    std::size_t m_length{0};
};

template <std::size_t SIZE>
inline Buffer_view make_buffer_view(std::array<std::uint8_t, SIZE> const& data)
{
    return Buffer_view(&data[0], SIZE);
}

class Mut_buffer_view
{
public:
    using value_type             = std::uint8_t;
    using pointer                = std::uint8_t*;
    using const_pointer          = std::uint8_t const*;
    using reference              = std::uint8_t&;
    using const_reference        = std::uint8_t const&;
    using const_iterator         = std::uint8_t const*;
    using iterator               = std::uint8_t*;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using reverse_iterator       = std::reverse_iterator<iterator>;
    using size_type              = std::size_t;
    using difference_type        = std::ptrdiff_t;

    Mut_buffer_view(std::uint8_t* data, std::size_t length)
        : m_data(data), m_length(length)
    {
    }

    Mut_buffer_view(Buffer_view buffer_view)
        : m_data(buffer_view.m_data), m_length(buffer_view.m_length)
    {
    }


    template <typename T>
    void set(T value, std::size_t offset)
    {
        T* field = reinterpret_cast<T*>(&m_data[offset]);
        *field   = value;
    }

    void copy_from(
        std::size_t dst_position, std::uint8_t const* src, std::size_t size)
    {
        std::memcpy(
            reinterpret_cast<void*>(m_data + dst_position),
            reinterpret_cast<void const*>(src),
            size);
    }

    template <
        typename T,
        typename = enable_if_t<!std::is_same<T, std::uint8_t>::value>>
    void copy_from(std::size_t dst_position, T const* src, std::size_t size)
    {
        std::memcpy(
            reinterpret_cast<void*>(m_data + dst_position),
            reinterpret_cast<void const*>(src),
            size * sizeof(T));
    }

    void reset(std::size_t pos, std::size_t length)
    {
        std::memset(
            reinterpret_cast<void*>(m_data + pos), 0,
            length * sizeof(std::uint8_t));
    }

    template <typename T>
    T* as_ptr(std::size_t offset)
    {
        return reinterpret_cast<T*>(&m_data[offset]);
    }

    template <typename T>
    T* as()   
    {
        return reinterpret_cast<T*>(&m_data[0]);
    }

    template <typename T>
    T* as(std::size_t offset)
    {
        return reinterpret_cast<T*>(m_data + offset);
    }

    operator Buffer_view() const { return Buffer_view{&m_data[0], m_length}; }

    iterator begin() const noexcept { return m_data; }
    iterator end() const noexcept { return m_data + m_length; }
    iterator cbegin() const noexcept { return begin(); }
    iterator cend() const noexcept { return end(); }

    reverse_iterator rbegin() const noexcept
    {
        return reverse_iterator(end());
    }

    reverse_iterator rend() const noexcept
    {
        return reverse_iterator(begin());
    }

    reverse_iterator crbegin() const noexcept { return rbegin(); }
    reverse_iterator crend() const noexcept { return rend(); }


    std::uint8_t& operator[] (std::size_t offset) noexcept
    {
        return m_data[offset];
    }

    std::size_t get_length() const { return m_length; }

private:
    std::uint8_t* m_data;
    std::size_t m_length;
};

template <std::size_t SIZE>
inline Mut_buffer_view
make_mut_buffer_view(std::array<std::uint8_t, SIZE>& data)
{
    return Mut_buffer_view(&data[0], SIZE);
}


class Buffer
{
public:
    Buffer()  = default;
    Buffer(std::size_t capacity)
        : m_buffer(std::vector<uint8_t>(capacity, 0)), m_position(0)
    {
    }

    void set_position(std::size_t position) { m_position = position; }

    std::size_t get_position() const { return m_position; }

    template <typename T>
    T const* position_ptr_as() const
    {
        return reinterpret_cast<T const*>(&m_buffer[m_position]);
    }

    template <typename T>
    T* position_ptr_as()
    {
        return reinterpret_cast<T*>(&m_buffer[m_position]);
    }

    template <typename T>
    T const* ptr_as(std::size_t position) const
    {
        return reinterpret_cast<T const*>(&m_buffer[position]);
    }

    template <typename T>
    T* ptr_as(std::size_t position)
    {
        return reinterpret_cast<T*>(&m_buffer[position]);
    }

    std::size_t remaining() const { return m_buffer.size() - m_position; }
    void transfer_to_start(std::size_t position, std::size_t length);

    void reset() { m_position = 0; }
    void reset(std::size_t remaining_position, std::size_t remaining_length)
    {
        reset();

        if (remaining_length > 0)
        {
            transfer_to_start(remaining_position, remaining_length);
        }
    }

    Buffer_view slice_from_position(std::size_t length)
    {
        return Buffer_view{&m_buffer[0], m_position + length};
    }

    Buffer_view slice_from_start(std::size_t length)
    {
        return Buffer_view{&m_buffer[0], length};
    }

    template <typename T>
    void set(T value, std::size_t offset)
    {
        T* field = reinterpret_cast<T*>(&m_buffer[offset]);
        *field   = value;
    }

    void copy_from(std::size_t dst_position, Buffer_view src)
    {
        std::uint8_t const* src_ptr = src.as<std::uint8_t>();
        std::copy(
            src_ptr, src_ptr + src.get_length(),
            m_buffer.begin() + dst_position);
    }

    void append(Buffer_view to_append)
    {
        // TODO: boundaries check may be
        copy_from(m_position, to_append);
    }

    operator Buffer_view()
    {
        return Buffer_view{&m_buffer[0], m_buffer.capacity()};
    }

    std::size_t capacity() const { return m_buffer.capacity(); };

private:
    std::vector<std::uint8_t> m_buffer;
    std::size_t m_position{0};
};

Buffer
to_buffer(Buffer_view view);

}  // namespace routing

#endif
