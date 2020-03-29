

list(APPEND ENGINE_GRPC_HEADERS
    "src/engine.pb.h"
    "src/engine.grpc.pb.h"
)

list (APPEND ENGINE_GRPC_SRC
    "src/engine.pb.cc"
    "src/engine.grpc.pb.cc"
    ${ENGINE_GRPC_HEADERS}
)


routing_library(
    TARGET 
        "engine_grpc"
    EXPORT_NAME 
        "engine::grpc"
    SOURCES 
        ${ENGINE_GRPC_SRC}
    PUBLIC_LIBRARIES 
        routing::common grpc++
    PUBLIC_INCLUDE_DIRS 
        "src"
    PRIVATE_INCLUDE_DIRS 
        "src"
    PRIVATE_COMPILE_DEFINITIONS
)
