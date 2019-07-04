

#ifndef ROUTING_BLOB_H
#define ROUTING_BLOB_H

#include "buffer.h"
#include <routing/stdext.h>
#include <routing/logger.h>

#include <string>
#include <cstdio>
#include <type_traits>
#include <spdlog/fmt/fmt.h>

#include <boost/optional.hpp>

namespace routing
{

class Serializing_blob;

class Blob 
{
public:
    Blob()
        : m_buffer(0) 
    {
    }

    Blob(int size)
        : m_buffer(size), m_offset(0)
    {
    }

    template <typename T>
    friend Blob& operator<<(Blob& blob, T value)
    {
        blob.m_buffer.set(value, blob.m_offset);
        blob.m_offset += sizeof(T);

        return blob;
    }

    void reset();

    friend Blob& operator<<(Blob& blob, Buffer_view value)
    {
        blob.m_buffer.copy_from(blob.m_offset, value);

        blob.m_offset += value.get_length();

        return blob;
    }

    std::size_t size() const { return m_offset; }
    std::size_t capacity() const { return m_buffer.capacity(); }
    std::size_t remaining() const { return capacity() - size(); }

    void save_to_file(std::string const& path);
private:
    friend class Serializing_blob;

    Buffer m_buffer;
    std::size_t m_offset{0};
};

class Serializing_blob
{
public:
    Serializing_blob() : m_filename(""), m_blob(0) 
    {
        m_logger = get_default_logger("Serializing_blob");
    }

    Serializing_blob(std::string const& filename, int buffer_size)
        : m_filename(filename), m_blob(buffer_size)
    {
        m_logger = get_default_logger("Serializing_blob");
    }

    ~Serializing_blob()
    {
        store_and_reset();
    }

    template <typename T>
    friend Serializing_blob& operator<<(Serializing_blob& blob, T value)
    {
        if (sizeof(T) > blob.m_blob.remaining())
        {
            blob.store_and_reset();
        }

        blob.m_blob << value;
        return blob;
    }

    friend Serializing_blob& operator<<(
        Serializing_blob& blob,
        Buffer_view value)
    {
        if (value.get_length() > blob.m_blob.remaining())
        {
            blob.store_and_reset();
        }

        if (value.get_length() > blob.m_blob.capacity())
        {
            return blob;
        }

        blob.m_blob << value;
        return blob;
    }

private:
    std::string m_filename;
    Blob m_blob;

    Logger_t m_logger;

    void store_and_reset();
};


}

#endif
