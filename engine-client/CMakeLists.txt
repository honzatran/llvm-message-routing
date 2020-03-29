

routing_exe(
    TARGET 
        "engine_client"
    SOURCES
        "src/main.cpp"
    PUBLIC_LIBRARIES
        routing::orc routing::engine::grpc
    PRIVATE_COMPILE_FLAGS
        ${DISABLE_RTTI_FLAG}
)
