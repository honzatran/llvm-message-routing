
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
        "engine"
    EXPORT_NAME 
        "engine"
    SOURCES 
        ${ROUTING_SRC}
    PUBLIC_LIBRARIES 
        routing::common
    PUBLIC_INCLUDE_DIRS 
        "include"
    PRIVATE_INCLUDE_DIRS 
        "src"
)

routing_test(
    TARGET 
        routing_engine_test
    SOURCES 
        ${ROUTING_TEST_SRC}
    PUBLIC_LIBRARIES 
        "routing::engine"
)
