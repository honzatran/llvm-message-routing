
list(APPEND PCM_PUBLIC_HEADERS
    )


set(OPCM_DIR "../3rd-party/opcm/pcm")


if (UNIX AND NOT APPLE)
    list(APPEND PCM_SRC
        ${OPCM_DIR}/msr.cpp
        ${OPCM_DIR}/cpucounters.cpp
        ${OPCM_DIR}/mmio.cpp
        ${OPCM_DIR}/client_bw.cpp
        ${OPCM_DIR}/utils.cpp
        ${OPCM_DIR}/pci.cpp

        ${PCM_PUBLIC_HEADERS})

# rely on Linux perf support (user needs CAP_SYS_ADMIN privileges), comment out to disable
    if (EXISTS /usr/include/linux/perf_event.h)
        list (APPEND PCM_PRIVATE_DEFINITION
            PCM_USE_PERF)

    endif()
endif()



list(APPEND PCM_SRC
    "src/pcm_wrapper.cpp"
)


routing_library(
    TARGET 
        "pcm"
    EXPORT_NAME 
        "pcm"
    SOURCES 
        ${PCM_SRC}
    PUBLIC_LIBRARIES 
        routing::common
    PUBLIC_INCLUDE_DIRS 
        "include"
    PRIVATE_INCLUDE_DIRS 
        "src"
        ${OPCM_DIR}/
        ${OPCM_DIR}/MacMSRDriver/
    PRIVATE_COMPILE_DEFINITIONS
        ${PCM_PRIVATE_DEFINITION}
)
