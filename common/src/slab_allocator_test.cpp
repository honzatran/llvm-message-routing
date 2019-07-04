

#include <routing/slab_allocator.h>
#include <routing/config.h>
#include <routing/logger.h>
#include <routing/testing.h>
#include <routing/stdext.h>
#include <routing/testing_malloc_allocator.h>

#include <gtest/gtest.h>
#include <iostream>

using namespace routing;
using namespace std;

void check_in_bounds(void* addr, std::pair<void*, size_t> slab)
{
    void* slab_start = slab.first;
    size_t slab_size = slab.second;

    EXPECT_GE(addr, slab_start);
    EXPECT_LT(addr, (void*) ((std::uint8_t *) slab_start + slab_size));
}

template <typename T, typename U>
void check_allocated_before(T* t_instance, U* u_instance)
{
    std::uint8_t* t_instance_arith_ptr = 
        reinterpret_cast<std::uint8_t*>(t_instance);

    std::uint8_t* u_instance_arith_ptr = 
        reinterpret_cast<std::uint8_t*>(u_instance);

    EXPECT_LE(t_instance_arith_ptr + sizeof(T), u_instance_arith_ptr);
}

class Mock_class
{
public:
    Mock_class(Function_ref<void()> on_destructor_callback)
        : m_on_destructor_callback(on_destructor_callback)
    {
    }

    Mock_class(Mock_class const& other) = delete;
    Mock_class& operator=(Mock_class const& other) = delete;

    Mock_class(Mock_class && other) = default;
    Mock_class& operator=(Mock_class && other) = default;

    ~Mock_class()
    {
        if (m_on_destructor_callback)
        {
            m_on_destructor_callback();
        }
    }

private:
    Function_ref<void()> m_on_destructor_callback;
};

class Slab_allocator_test : public ::testing::Test
{
public:
    static void SetUpTestCase() 
    {
        auto config_options = 
        {
            get_logger_options_description(),
        };

        Config config = 
            routing::parse_args(config_options, TEST_CONFIG_PATH);

        spdlog::drop("logger");
        init_logger(config);
    }

};

class Tracking_specific_allocator_test : public ::testing::Test
{
public:
    static void SetUpTestCase() 
    {
        auto config_options = 
        {
            get_logger_options_description(),
        };

        Config config = 
            routing::parse_args(config_options, TEST_CONFIG_PATH);

        spdlog::drop("logger");
        init_logger(config);
    }

};

// class Logging_malloc_allocator
// {
// public:
//     Logging_malloc_allocator(
//             std::shared_ptr<spdlog::logger> logger,
//             std::vector<std::pair<void*, size_t>>* tracked_slabs)
//         : m_logger(logger), m_tracked_slabs(tracked_slabs)
//     {
//     }
//
//     void* allocate(std::size_t size, size_t alignment)
//     {
//         if (m_logger)
//         {
//             m_logger->info(
//                     "malloc with size {} alignment {}", 
//                     size, 
//                     alignment);
//         }
//
//
//         void* slab = malloc(size);
//         m_tracked_slabs->push_back(std::make_pair(slab, size));
//
//         return slab;
//     }
//
//     void deallocate(void const *ptr, size_t size)
//     {
//         if (m_logger)
//         {
//             m_logger->info("deallocate ptr {} size {}", ptr, size);
//         }
//
//         auto tracked_pair = std::make_pair(const_cast<void*>(ptr), size);
//
//         auto it = std::find(
//                 m_tracked_slabs->begin(),
//                 m_tracked_slabs->end(),
//                 tracked_pair);
//
//         EXPECT_NE(it, m_tracked_slabs->end());
//
//         if (it != m_tracked_slabs->end())
//         {
//             m_tracked_slabs->erase(it);
//         }
//
//         free(const_cast<void *>(ptr));
//     }
//     
//     std::pair<void*, size_t> get_slab(std::size_t slab_index)
//     {
//         return m_tracked_slabs->at(slab_index);
//     }
//
// private:
//     std::shared_ptr<spdlog::logger> m_logger;
//
//     std::vector<std::pair<void*, size_t>>* m_tracked_slabs;
// };

TEST_F(Slab_allocator_test, basic_test)
{
    auto logger = get_default_logger();
    vector<pair<void*, size_t>> tracked_slabs;

    Logging_malloc_allocator allocator(logger, &tracked_slabs);
    {
        Slab_allocator<Logging_malloc_allocator> slab_allocator(allocator);

        void* small_data = slab_allocator.allocate(sizeof(int), 8);

        EXPECT_EQ(1ULL, tracked_slabs.size());

        check_in_bounds(small_data, tracked_slabs[0]);
    }

    EXPECT_EQ(0UL, tracked_slabs.size());
}

TEST_F(Slab_allocator_test, alloc_two_slabs)
{
    auto logger = get_default_logger();
    vector<pair<void*, size_t>> tracked_slabs;

    Logging_malloc_allocator allocator(logger, &tracked_slabs);
    {
        Slab_allocator<Logging_malloc_allocator, 128> slab_allocator(allocator);

        void* first_blob = slab_allocator.allocate(120, 8);
        void* second_blob = slab_allocator.allocate(32, 8);

        EXPECT_EQ(2ULL, tracked_slabs.size());

        check_in_bounds(first_blob, tracked_slabs[0]);
        check_in_bounds(second_blob, tracked_slabs[1]);
    }

    EXPECT_EQ(0UL, tracked_slabs.size());
}

TEST_F(Slab_allocator_test, many_allocations_single_slab)
{
    auto logger = get_default_logger();
    vector<pair<void*, size_t>> tracked_slabs;

    Logging_malloc_allocator allocator(logger, &tracked_slabs);
    {
        Slab_allocator<Logging_malloc_allocator> slab_allocator(allocator);

        void* prev = nullptr;

        for (std::size_t i = 0; i < 100; ++i)
        {
            void* data = slab_allocator.allocate(11, 8);

            EXPECT_GE(data, prev);
            prev = reinterpret_cast<std::uint8_t*>(data) + 11;
        }
    }

    EXPECT_EQ(0UL, tracked_slabs.size());
}

TEST_F(Slab_allocator_test, big_allocation_slab)
{
    auto logger = get_default_logger();
    vector<pair<void*, size_t>> tracked_slabs;

    Logging_malloc_allocator allocator(logger, &tracked_slabs);
    {
        Slab_allocator<Logging_malloc_allocator, 128> slab_allocator(allocator);

        void* big_slab = slab_allocator.allocate(1024, 8);

        EXPECT_EQ(1UL, tracked_slabs.size());

        check_in_bounds(big_slab, tracked_slabs[0]);
        EXPECT_LE(1024UL, tracked_slabs[0].second);
    }

    EXPECT_EQ(0UL, tracked_slabs.size());
}

TEST_F(Tracking_specific_allocator_test, basic_test)
{
    auto logger = get_default_logger();
    vector<pair<void*, size_t>> tracked_slabs;
    Logging_malloc_allocator allocator(logger, &tracked_slabs);

    int destructed_mock_classes = 0;

    auto destructor_callback
        = [&destructed_mock_classes]() mutable { destructed_mock_classes++; };

    {
        Slab_allocator<Logging_malloc_allocator> slab_allocator(allocator);

        Tracking_specific_allocator<int, Logging_malloc_allocator>
            int_allocator(&slab_allocator);

        Tracking_specific_allocator<Mock_class, Logging_malloc_allocator>
            mock_class_allocator(&slab_allocator);

        int* value = int_allocator.create(42);

        EXPECT_EQ(42, *value);

        int* value1 = int_allocator.create(4242);

        check_allocated_before(value, value1);

        EXPECT_EQ(42, *value);
        EXPECT_EQ(4242, *value1);

        Mock_class* mock_class
            = mock_class_allocator.create(destructor_callback);

        check_allocated_before(value1, mock_class);

        Mock_class* mock_class1
            = mock_class_allocator.create(destructor_callback);

        check_allocated_before(mock_class, mock_class1);
    }

    EXPECT_EQ(0UL, tracked_slabs.size());
    EXPECT_EQ(2, destructed_mock_classes);
}




