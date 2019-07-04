
#ifndef ROUTING_SLAB_ALLOCATOR_H
#define ROUTING_SLAB_ALLOCATOR_H

#include <cstdlib>
#include <utility>
#include <memory>
#include <vector>

namespace routing
{

// template <std::size_t SLAB_SIZE>
// class Slab
// {
// public:
//     Slab()
//     {
//         m_slab_ptr = new void
//     }
//
//     Slab(Slab const& other) = delete;
//     Slab& operator=(Slab const& other) = delete;
//
//     Slab(Slab&& other) = delete;
//     Slab& operator=(Slab&& other) = delete;
//
// private:
//     void* m_slab_ptr;
//     void* m_current_ptr;
// };
//

inline std::uintptr_t 
align_addr(void const* addr, std::size_t alignment)
{
    // assert (aligment && is_pow_2(alignment);
    // assert ((uintptr_t) addr + alignment >= (uintptr_t) addr)
    //
    return (((uintptr_t) addr + alignment - 1) 
            & ~((uintptr_t) (alignment - 1)));
}

inline size_t 
alignment_adjustment(void const* ptr, size_t alignment)
{
    return align_addr(ptr, alignment) - (uintptr_t) ptr;
}

class Malloc_allocator
{
public:
    void* allocate(std::size_t size, size_t /*alignment*/)
    {
        return malloc(size);
    }

    void deallocate(void const *ptr, size_t /*size*/)
    {
        free(const_cast<void *>(ptr));
    }
private:
};

template <typename ALLOCATOR = Malloc_allocator, std::size_t SLAB_SIZE = 4096>
class Slab_allocator
{ 
public:
    template <typename O_ALLOCATOR>
    Slab_allocator(O_ALLOCATOR&& allocator) : 
        m_allocator(std::forward<O_ALLOCATOR>(allocator)) { }


    ~Slab_allocator()
    {
        deallocate_all_slabs();
        deallocate_custom_size_slabs();
    }

    void* allocate(size_t size, size_t alignment)
    {
        m_allocated_bytes += size;

        size_t adjusment = alignment_adjustment(m_cur_ptr, alignment);

        if (fit_into_current_slab(adjusment, size))
        {
            std::uint8_t* aligned_ptr = m_cur_ptr + adjusment;
            m_cur_ptr = aligned_ptr + size;

            return aligned_ptr;
        }

        size_t padded_size = alignment + size - 1;

        if (padded_size > SLAB_SIZE)
        {
            // add support for allocating sizes bigger than SLAB_SIZE
            void* custom_slab = add_custom_slab(padded_size);
            std::uint8_t* aligned_ptr = 
                (std::uint8_t *) align_addr(custom_slab, alignment);

            return aligned_ptr;
        }
        
        start_new_slab();
        // assert (size + adjusment > size)
        std::uint8_t* aligned_ptr = 
            (std::uint8_t *) align_addr(m_cur_ptr, alignment);

        m_cur_ptr = aligned_ptr + size;

        return aligned_ptr;
    }

private:
    ALLOCATOR m_allocator;

    std::vector<void*> m_slabs;
    std::vector<std::pair<void*, size_t>> m_custom_size_slabs;

    std::uint8_t* m_cur_ptr = nullptr;
    std::uint8_t* m_end = nullptr;
    std::size_t m_allocated_bytes = 0;

    void deallocate_all_slabs()
    {
        for (void* slab : m_slabs)
        {
            m_allocator.deallocate(slab, SLAB_SIZE);
        }
    }

    void deallocate_custom_size_slabs()
    {
        for (auto custom_slab_pair : m_custom_size_slabs)
        {
            void* custom_slab = custom_slab_pair.first;
            size_t size = custom_slab_pair.second;
            
            m_allocator.deallocate(custom_slab, size);
        }
    }

    void* add_custom_slab(std::size_t custom_slab_size)
    {
        void* custom_slab = m_allocator.allocate(custom_slab_size, 0);
        m_custom_size_slabs.push_back(
                std::make_pair(custom_slab, custom_slab_size));

        return custom_slab;
    }

    void start_new_slab()
    {
        void* new_slab = m_allocator.allocate(SLAB_SIZE, 0);

        m_slabs.push_back(new_slab);
        m_cur_ptr = reinterpret_cast<std::uint8_t *>(new_slab);
        m_end = m_cur_ptr + SLAB_SIZE;
    }

    bool fit_into_current_slab(std::size_t adjusment, std::size_t size)
    {
        return adjusment + size < size_t(m_end - m_cur_ptr);
    }
};

template <
    typename T, 
    typename MALLOC = Malloc_allocator, 
    std::size_t SLAB_SIZE = 4096>
class Tracking_specific_allocator
{
public:
    using Slab_allocator_t = Slab_allocator<MALLOC, SLAB_SIZE>;

    Tracking_specific_allocator() = default;

    // slab_allocator
    Tracking_specific_allocator(Slab_allocator_t* slab_allocator)
        : m_slab_allocator(slab_allocator)
    {
    }

    ~Tracking_specific_allocator()
    {
        deallocate_tracked_instances();
    }

    Tracking_specific_allocator(Tracking_specific_allocator const& other) = 
        delete;

    Tracking_specific_allocator& operator=(
            Tracking_specific_allocator const& other) = delete;

    Tracking_specific_allocator(Tracking_specific_allocator&& other) = default;

    Tracking_specific_allocator& operator=(
            Tracking_specific_allocator&& other) = default;

    template <typename ...ARGS>
    T* create(ARGS&&... args)
    {
        void* instance_storage = 
            m_slab_allocator->allocate(sizeof(T), alignof(T));

        T* new_instance = 
            new (instance_storage) T(std::forward<ARGS>(args)...);

        m_tracked_instances.push_back(new_instance);

        return new_instance;
    }

    T* allocate(std::size_t count)
    {
        void* instance_storage =
            m_slab_allocator->allocate(count * sizeof(T), alignof(T));

        return static_cast<T*>(instance_storage);
    }

private:
    Slab_allocator<MALLOC, SLAB_SIZE>* m_slab_allocator;

    std::vector<T*> m_tracked_instances;

    void deallocate_tracked_instances()
    {
        for (T* tracked_instance : m_tracked_instances)
        {
            tracked_instance->~T();
        }
    }
};

// tracking allocator predicates
template <typename TRACKING_ALLOCATOR>

struct Tracking_allocator_traits
{
};

template <
    typename T, 
    typename MALLOC, 
    std::size_t SLAB_SIZE>
struct Tracking_allocator_traits<
            Tracking_specific_allocator<T, MALLOC, SLAB_SIZE>>
{
    using tracked_type = T;
};

template <typename TRACKING_ALLOCATOR, typename T>
struct is_tracked_type : std::false_type
{
};

template <typename TRACKING_ALLOCATOR>
struct is_tracked_type
    <
        TRACKING_ALLOCATOR,
        typename Tracking_allocator_traits<TRACKING_ALLOCATOR>::tracked_type
    > : std::true_type
{
};

template <typename T>
class Slab_ptr
{
public:
    Slab_ptr() = default;

    template <typename MALLOC, std::size_t SLAB_SIZE, typename... ARGS>
    Slab_ptr(Slab_allocator<MALLOC, SLAB_SIZE>* slab_allocator, ARGS&&... args)
    {
        void* instance_storage = 
            slab_allocator->allocate(sizeof(T), alignof(T));

        m_instance = 
            new (instance_storage) T(std::forward<ARGS>(args)...);
    }

    Slab_ptr(Slab_ptr const& other) = delete;
    Slab_ptr& operator=(Slab_ptr const& other) = delete;

    Slab_ptr(Slab_ptr&& other) 
        : m_instance(other.m_instance)
    {
        other.m_instance = nullptr;
    }

    Slab_ptr& operator=(Slab_ptr&& other) 
    {
        if (&other != this)
        {
            m_instance = other.m_instance;
            other.m_instance = nullptr;
        }

        return *this;
    }
    
    ~Slab_ptr()
    {
        if (m_instance != nullptr)
        {
            m_instance->~T();
        }
    }

    T* operator->() { return m_instance; }

    T const* operator->() const { return m_instance; }

    T& operator*() { return *m_instance; }

    T const& operator*() const { return *m_instance; }

    T* get()
    {
        return m_instance;
    }

    T const* get() const
    {
        return m_instance;
    }


private:
    T* m_instance;

};

/// the default slab allocator, using malloc 
using Slab_allocator_t = Slab_allocator<>;

}

#endif
