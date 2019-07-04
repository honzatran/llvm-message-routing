
#ifndef ROUTING_POINTER_UTILS_H
#define ROUTING_POINTER_UTILS_H

namespace routing
{
template <typename T>
typename std::decay<T>::type*
get_ptr(void* ptr, std::size_t offset)
{
    std::uint8_t* ar_ptr = reinterpret_cast<std::uint8_t*>(ptr);

    return reinterpret_cast<typename std::decay<T>::type*>(ar_ptr + offset);
}
};

#endif
