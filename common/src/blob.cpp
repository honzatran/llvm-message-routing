

#include <routing/blob.h>
#include <routing/file_util.h>

#include <cstdio>

using namespace routing;

void Blob::reset()
{
    m_buffer.reset();
    m_offset = 0;
}

void Blob::save_to_file(std::string const& path)
{
    Open_file wrapper(path, "wb");

    Buffer_view view = m_buffer.slice_from_position(m_offset);

    fwrite(
        view.as<void>(),
        sizeof(std::uint8_t),
        view.get_length(),
        wrapper.get_file());

    fflush(wrapper.get_file());
}

void Serializing_blob::store_and_reset()
{
    if (m_blob.m_offset == 0)
    {
        return;
    }

    Open_file wrapper(m_filename, "ab");

    m_logger->error_if(
        wrapper.get_file() == nullptr, "Serializing to file {}", m_filename);

    Buffer_view view = m_blob.m_buffer.slice_from_position(m_blob.m_offset);

    fwrite(
        view.as<void>(),
        sizeof(std::uint8_t),
        view.get_length(),
        wrapper.get_file());

    m_blob.reset();
}
