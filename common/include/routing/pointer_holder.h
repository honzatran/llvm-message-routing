

#ifndef ROUTING_POINTER_HOLDER_H
#define ROUTING_POINTER_HOLDER_H

namespace routing
{

template <typename T>
class Pointer_holder
{
public:
    Pointer_holder(T* ptr) : m_ptr(ptr) {}

    using base_t = Pointer_holder<T>;

protected:

    T& get_instance() 
    {
        return *m_ptr;
    }

private:
    T* const m_ptr;
};

}

#endif
