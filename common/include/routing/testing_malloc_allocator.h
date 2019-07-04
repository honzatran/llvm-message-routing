

#ifndef ROUTING_TESTING_MALLOC_ALLOCATOR_H
#define ROUTING_TESTING_MALLOC_ALLOCATOR_H

#include <routing/logger.h>

#include <cstdlib>
#include <utility>
#include <gtest/gtest.h>

class Logging_malloc_allocator
{
public:
    Logging_malloc_allocator(
            std::shared_ptr<spdlog::logger> logger,
            std::vector<std::pair<void*, size_t>>* tracked_slabs)
        : m_logger(std::move(logger)), m_tracked_slabs(tracked_slabs)
    {
    }

    void* allocate(std::size_t size, size_t alignment)
    {
        if (m_logger)
        {
            m_logger->info(
                    "malloc with size {} alignment {}", 
                    size, 
                    alignment);
        }


        void* slab = malloc(size);
        m_tracked_slabs->push_back(std::make_pair(slab, size));

        return slab;
    }

    void deallocate(void const *ptr, size_t size)
    {
        if (m_logger)
        {
            m_logger->info("deallocate ptr {} size {}", ptr, size);
        }

        auto tracked_pair = std::make_pair(const_cast<void*>(ptr), size);

        auto it = std::find(
                m_tracked_slabs->begin(),
                m_tracked_slabs->end(),
                tracked_pair);

        EXPECT_NE(it, m_tracked_slabs->end());

        if (it != m_tracked_slabs->end())
        {
            m_tracked_slabs->erase(it);
        }

        free(const_cast<void *>(ptr));
    }
    
    std::pair<void*, size_t> get_slab(std::size_t slab_index)
    {
        return m_tracked_slabs->at(slab_index);
    }

private:
    std::shared_ptr<spdlog::logger> m_logger;

    std::vector<std::pair<void*, size_t>>* m_tracked_slabs;
};


#endif
