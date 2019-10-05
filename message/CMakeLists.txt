
list(APPEND ROUTING_PUBLIC_HEADERS
    )

list(APPEND ROUTING_SRC
    "src/message.cpp"
    ${FASTLANE_PUBLIC_HEADERS}
)

list (APPEND ROUTING_TEST_SRC 
    "src/message_key_test.cpp"
    "src/message_test.cpp"
)

routing_library(
    TARGET 
        "message"
    EXPORT_NAME 
        "message"
    SOURCES 
        ${ROUTING_SRC}
    PUBLIC_LIBRARIES 
        routing::common
    PUBLIC_INCLUDE_DIRS 
        "include"
    PRIVATE_INCLUDE_DIRS 
        "src"
    PRIVATE_COMPILE_DEFINITIONS
)

routing_test(
    TARGET 
        routing_message_test
    SOURCES 
        ${ROUTING_TEST_SRC}
    PUBLIC_LIBRARIES 
        "routing::message"
)

list (APPEND ROUTING_MESSAGE_BENCHMARK_SRC 
    "src/message_benchmark.cpp"
)


routing_benchmark(
    TARGET
        routing_message_benchmark
    SOURCES
        ${ROUTING_MESSAGE_BENCHMARK_SRC}
    PUBLIC_LIBRARIES
        "routing::message"
)


