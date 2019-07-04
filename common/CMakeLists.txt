
set (COMMON "routing_common")
set (COMMON_TEST "common_test")

list(APPEND COMMON_PUBLIC_HEADERS
        "include/routing/algorithm_util.h"
        "include/routing/application.h"
        "include/routing/bits_util.h"
        "include/routing/blob.h"
        "include/routing/buffer.h"
        "include/routing/call_counter.h"
        "include/routing/config.h"
        "include/routing/debug.h"
        "include/routing/decimal.h"
        "include/routing/detect.h"
        "include/routing/fatal_signal_handlers.h"
        "include/routing/file_util.h"
        "include/routing/fmt.h"
        "include/routing/functional.h"
        "include/routing/hooks.h"
        "include/routing/idle_strategy.h"
        "include/routing/instance_container.h"
        "include/routing/latches.h"
        "include/routing/logger.h"
        "include/routing/looping_state.h"
        "include/routing/meta.h"
        "include/routing/operation.h"
        "include/routing/operation_register.h"
        "include/routing/overload.h"
        "include/routing/pointer_holder.h"
        "include/routing/pointer_utils.h"
        "include/routing/poller.h"
        "include/routing/random_utils.h"
        "include/routing/resource_state.h"
        "include/routing/ring_buffer.h"
        "include/routing/slab_allocator.h"
        "include/routing/span.h"
        "include/routing/stacktrace.h"
        "include/routing/stacktrace_printer.h"
        "include/routing/stdext.h"
        "include/routing/string_utils.h"
        "include/routing/sys_util.h"
        "include/routing/synchronized.h"
        "include/routing/tcp_socket.h"
        "include/routing/tcp_util.h"
        "include/routing/testing.h"
        "include/routing/testing_malloc_allocator.h"
        "include/routing/thread_factory.h"
        "include/routing/throttle.h"
        "include/routing/time_mixin.h"
        "include/routing/time_util.h"
        "include/routing/type_traits.h"
    )

list(APPEND COMMON_SRC 
        "src/application.cpp"
        "src/blob.cpp"
        "src/buffer.cpp"
        "src/config.cpp"
        "src/fatal_signal_handlers.cpp"
        "src/file_util.cpp"
        "src/hooks.cpp"
        "src/logger.cpp"
        "src/operation_register.cpp"
        "src/poller.cpp"
        "src/poller_providers.cpp"
        "src/stacktrace.cpp"
        "src/stacktrace_printer.cpp"
        "src/string_utils.cpp"
        "src/tcp_socket.cpp"
        "src/tcp_util.cpp"
        "src/thread_factory.cpp"
        ${COMMON_PUBLIC_HEADERS}
    )

list (APPEND COMMON_TEST_SRC 
        "src/bits_util_test.cpp"
        "src/buffer_test.cpp"
        "src/config_test.cpp"
        "src/debug_test.cpp"
        "src/hooks_test.cpp"
        "src/instance_container_test.cpp"
        "src/looping_state_test.cpp"
        "src/operation_register_test.cpp"
        "src/overload_test.cpp"
        "src/ring_buffer_test.cpp"
        "src/slab_allocator_test.cpp"
        "src/stdext_test.cpp"
        "src/string_utils_test.cpp"
        "src/synchronized_test.cpp"
        "src/tcp_socket_test.cpp"
        "src/time_mixin_test.cpp"
        "src/algorithm_util_test.cpp"
        "src/sorted_small_array_test.cpp"
        "src/throttle_test.cpp"
        "src/matcher_test.cpp"
        "src/functional_test.cpp"
        "src/type_traits_test.cpp"
        "src/poller_test.cpp"
        "src/poller_providers_test.cpp"
    )

list(APPEND COMMON_BENCHMARK_SRC
        "src/benchmark.cpp")

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/include/routing/configuration.h.in 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/routing/configuration.h
    @ONLY)

routing_library(
    TARGET 
        ${COMMON}
    EXPORT_NAME 
        "common"
    SOURCES 
        ${COMMON_SRC}
    PUBLIC_LIBRARIES 
        ${Boost_LIBRARIES} 
        pthread 
        absl::flat_hash_map
    PUBLIC_INCLUDE_DIRS
        "include"
    PRIVATE_INCLUDE_DIRS 
        "src"
)

routing_test(
    TARGET 
        common_test
    SOURCES 
        ${COMMON_TEST_SRC}
    PUBLIC_LIBRARIES 
        routing::common
)

routing_benchmark(
    TARGET
        common_benchmark
    SOURCES
        ${COMMON_BENCHMARK_SRC}
    PUBLIC_LIBRARIES
        routing::common
)

# target_compile_options(${COMMON_TEST}_bin PUBLIC
#     "-fno-omit-frame-pointer" "-fsanitize=address")
#
# set_property(TARGET ${COMMON_TEST}_bin APPEND_STRING 
#    PROPERTY LINK_FLAGS "-fno-omit-frame-pointer -fsanitize=address")


