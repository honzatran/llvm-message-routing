

#include <routing/buffer.h>

#include <algorithm>

using namespace std;


void 
routing::Buffer::transfer_to_start(
        std::size_t position, 
        std::size_t length)
{
    std::copy_n(
            m_buffer.begin() + position, 
            length, 
            m_buffer.begin() + m_position);

    m_position += length;
}

routing::Buffer
routing::to_buffer(routing::Buffer_view view)
{
    Buffer buffer(view.get_length());
    buffer.copy_from(0, view);

    return buffer;
}
