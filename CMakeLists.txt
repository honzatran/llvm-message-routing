
project(MESSAGE_ROUTING)

function (init_google_test)
    configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
    execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download )
    if(result)
        message(FATAL_ERROR "CMake step for googletest failed: ${result}")
    endif()
    execute_process(COMMAND ${CMAKE_COMMAND} --build .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download )
    if(result)
        message(FATAL_ERROR "Build step for googletest failed: ${result}")
    endif()

# Prevent overriding the parent project's compiler/linker
# settings on Windows
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This defines
# the gtest and gtest_main targets.
    add_subdirectory(${CMAKE_BINARY_DIR}/googletest-src
        ${CMAKE_BINARY_DIR}/googletest-build
        EXCLUDE_FROM_ALL)

    if (CMAKE_VERSION VERSION_LESS 2.8.11)
        include_directories(BEFORE SYSTEM
        "${gtest_SOURCE_DIR}/include" "${gmock_SOURCE_DIR}/include")
    else()
        # target_include_directories(gmock_main SYSTEM BEFORE INTERFACE
        #     "${gtest_SOURCE_DIR}/include" "${gmock_SOURCE_DIR}/include")
    endif()

endfunction()

set(LIB_INSTALL "lib")
set(BIN_INSTALL "bin")

function (install_binary target)
    install(TARGETS ${target} RUNTIME DESTINATION ${BIN_INSTALL})
endfunction()

function (install_lib target)
    set(LIB_VERSION $ENV{GIT_SHORT_HASH})

    if (NOT ${LIB_VERSION} STREQUAL "")
        set_target_properties(${target} PROPERTIES VERSION ${LIB_VERSION})
    endif()

    install(TARGETS ${target} 
            LIBRARY DESTINATION ${LIB_INSTALL})
endfunction()

function (link_address_sanitizer target)
    target_compile_options(${target} PUBLIC
        "-fno-omit-frame-pointer" "-fsanitize=address")

    set_property(TARGET ${target} APPEND_STRING 
       PROPERTY LINK_FLAGS "-fno-omit-frame-pointer -fsanitize=address")
endfunction()

function (run_clang_tidy target)
    set_target_properties(${target} PROPERTIES CXX_CLANG_TIDY "${DO_CLANG_TIDY}")
endfunction()


function(install_headers header_dir)
    install(DIRECTORY ${header_dir}/
        DESTINATION "include/")
endfunction()

enable_testing()

cmake_minimum_required(VERSION 2.8.12)

set(PATH_LIBCPP "Default" CACHE STRING 
	"Determine the prefix to libstdc++, by default uses the standart on path")

set(RHEL_VERSION "rhel7" CACHE STRING 
    "RHEL version, by default set to rhel7")


option(BUILD_TESTING "Determine whether tests are build" ON)
option(USE_ADDRESS_SANITIZER "Determine whether tests are build" ON)
option(USE_LIBUNWIND "ENABLE LIBUWIND STACKTRACES" OFF)
option(RUN_CLANG_TIDY "ENABLE CLANG TIDY" OFF)

function (routing_test) 
    cmake_parse_arguments(ROUTING_TEST
        "DISABLE_SANITIZER"
        "TARGET"
        "SOURCES;PUBLIC_LIBRARIES;PRIVATE_COMPILE_FLAGS;PUBLIC_INCLUDE_DIRS"
        ${ARGN}
    )

    if (BUILD_TESTING)
        set (_NAME ${ROUTING_TEST_TARGET})
        string(TOUPPER ${_NAME} _UPPER_NAME)

        add_executable(${_NAME}_bin 
            ${ROUTING_TEST_SOURCES} ${PROJECT_SOURCE_DIR}/testing/test_main.cpp)

        target_compile_options(${_NAME}_bin 
            PRIVATE ${ROUTING_TEST_PRIVATE_COMPILE_FLAGS})

        target_link_libraries(${_NAME}_bin 
            gmock_main ${ROUTING_TEST_PUBLIC_LIBRARIES})

        target_include_directories(${_NAME}_bin
            PUBLIC ${ROUTING_TEST_PUBLIC_INCLUDE_DIRS})

        add_test(NAME ${_NAME} COMMAND ${_NAME}_bin  --gtest_output=xml:${ROUTING_TEST_TARGET}/results.xml
             WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

         if (USE_ADDRESS_SANITIZER AND NOT ROUTING_TEST_DISABLE_SANITIZER)
             link_address_sanitizer(${_NAME}_bin)
         endif()

        if (RUN_CLANG_TIDY)
            run_clang_tidy(${_NAME}_bin)
        endif()

    endif()

endfunction()

function (routing_benchmark) 
    cmake_parse_arguments(ROUTING_BENCHMARK
        ""
        "TARGET"
        "SOURCES;PUBLIC_LIBRARIES;PRIVATE_COMPILE_FLAGS;PUBLIC_INCLUDE_DIRS"
        ${ARGN}
    )

    if (BUILD_TESTING)
        set (_NAME ${ROUTING_BENCHMARK_TARGET})
        string(TOUPPER ${_NAME} _UPPER_NAME)

        set (_BENCHMARK_NAME ${ROUTING_BENCHMARK_TARGET}_bin)

        add_executable(${_BENCHMARK_NAME}
            ${ROUTING_BENCHMARK_SOURCES} ${PROJECT_SOURCE_DIR}/testing/test_benchmark_main.cpp)

        target_compile_options(${_BENCHMARK_NAME} 
            PRIVATE ${ROUTING_BENCHMARK_PRIVATE_COMPILE_FLAGS})

        target_link_libraries(${_BENCHMARK_NAME}
            gmock_main ${ROUTING_BENCHMARK_PUBLIC_LIBRARIES})

        target_link_libraries(${_BENCHMARK_NAME}
            benchmark ${ROUTING_BENCHMARK_PUBLIC_LIBRARIES})

        if (RUN_CLANG_TIDY)
            run_clang_tidy(${_NAME}_bin)
        endif()

    endif()

endfunction()

function(routing_library)
    cmake_parse_arguments(ROUTING_LIB
        "IGNORE_INSTALL"
        "TARGET;EXPORT_NAME"
        "SOURCES;PUBLIC_LIBRARIES;PRIVATE_COMPILE_FLAGS;PUBLIC_INCLUDE_DIRS;PRIVATE_INCLUDE_DIRS;PRIVATE_COMPILE_DEFINITIONS"
        ${ARGN}
    )

    set(_NAME ${ROUTING_LIB_TARGET})
    string(TOUPPER ${_NAME} _UPPER_NAME)

    add_library(${_NAME} SHARED ${ROUTING_LIB_SOURCES})

    target_compile_options(
        ${_NAME} PRIVATE ${ROUTING_LIB_PRIVATE_COMPILE_FLAGS})

    target_link_libraries(
        ${_NAME} PUBLIC ${ROUTING_LIB_PUBLIC_LIBRARIES})

    target_include_directories(${_NAME}
        PUBLIC ${ROUTING_LIB_PUBLIC_INCLUDE_DIRS}
        PRIVATE ${ROUTING_LIB_PRIVATE_INCLUDE_DIRS})

    if (ROUTING_LIB_EXPORT_NAME)
        add_library(routing::${ROUTING_LIB_EXPORT_NAME} ALIAS ${_NAME})
    endif()

    target_compile_definitions(${_NAME} 
        PRIVATE ${ROUTING_LIB_PRIVATE_COMPILE_DEFINITIONS})

    if (NOT ROUTING_LIB_IGNORE_INSTALL)
        install_lib(${_NAME})
        install_headers(${ROUTING_LIB_PUBLIC_INCLUDE_DIRS})
    endif()


endfunction()

function(routing_exe)
    cmake_parse_arguments(ROUTING_EXE
        "NOT_INSTALL;ADDRESS_SANITIZER"
        "TARGET"
        "SOURCES;PUBLIC_LIBRARIES;PRIVATE_LIBRARIES;INCLUDE_DIRS;PRIVATE_COMPILE_FLAGS"
        ${ARGN}
    )

    set(_NAME ${ROUTING_EXE_TARGET})
    string(TOUPPER ${_NAME} _UPPER_NAME)

    add_executable(${_NAME} ${ROUTING_EXE_SOURCES})

    target_compile_options(
        ${_NAME} PRIVATE ${ROUTING_EXE_PRIVATE_COMPILE_FLAGS})

    target_link_libraries(
        ${_NAME} PUBLIC ${ROUTING_EXE_PUBLIC_LIBRARIES})

    target_include_directories(${_NAME}
        PRIVATE ${ROUTING_LIB_INCLUDE_DIRS})

    if (NOT ROUTING_EXE_IGNORE_INSTALL)
        install_binary(${ROUTING_EXE_TARGET})
    endif()

     if (USE_ADDRESS_SANITIZER)
         link_address_sanitizer(${_NAME})
     endif()

    if (RUN_CLANG_TIDY)
        run_clang_tidy(${_NAME})
    endif()

endfunction()

file(MAKE_DIRECTORY logs)
file(MAKE_DIRECTORY config)

set(CMAKE_CXX_FLAGS "-g -Wall")
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if (RUN_CLANG_TIDY)
    find_program(
        CLANG_TIDY_EXE
        NAMES "clang-tidy"
        DOC "Path to clang-tidy")

    if (NOT CLANG_TIDY_EXE)
        message(WARNING "clang-tidy not found, clang-tidy won't be executed")
        set(RUN_CLANG_TIDY OFF)
    else (NOT CLANG_TIDY_EXE)
        message(STATUS  "clang-tidy found : ${CLANG_TIDY_EXE}")
        set(DO_CLANG_TIDY "${CLANG_TIDY_EXE}" "-fix")
    endif()
endif()

add_custom_target(link_config_target ALL
                      COMMAND ${CMAKE_COMMAND} -E create_symlink 
                      ${CMAKE_CURRENT_SOURCE_DIR}/config
                      ${CMAKE_BINARY_DIR}/config) 

add_custom_target(link_resources_target ALL
                      COMMAND ${CMAKE_COMMAND} -E create_symlink 
                      ${CMAKE_CURRENT_SOURCE_DIR}/resources
                      ${CMAKE_BINARY_DIR}/resources) 

add_custom_target(link_logs_target ALL
                      COMMAND ${CMAKE_COMMAND} -E create_symlink 
                      ${CMAKE_CURRENT_SOURCE_DIR}/logs
                      ${CMAKE_BINARY_DIR}/logs) 

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/3rd-party/cmake)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(Boost_NO_BOOST_CMAKE ON CACHE BOOL "" FORCE)

find_package(Boost 1.70 COMPONENTS program_options thread REQUIRED)

init_google_test()

# configure_file(
#     ${CMAKE_CURRENT_SOURCE_DIR}/src/configuration.h.in 
#     ${CMAKE_CURRENT_SOURCE_DIR}/src/configuration.h
#     @ONLY)

set(BENCHMARK_ENABLE_TESTING 
    OFF 
    CACHE BOOL 
    "Supressing google benchmark test" 
    FORCE)

set (CMAKE_POSITION_INDEPENDENT_CODE TRUE)

set(absl_gtest_src_dir ${CMAKE_BINARY_DIR}/googletest-src)
set(absl_gtest_build_dir ${CMAKE_BINARY_DIR}/googletest-build)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/3rd-party/google/benchmark)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/3rd-party/google/abseil)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/3rd-party/spdlog-1.3.1)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/common)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/pcm-wrapper)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/message)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/engine)

add_custom_target(copy_compilation_db ALL
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
  ${CMAKE_BINARY_DIR}/compile_commands.json
  ${CMAKE_CURRENT_SOURCE_DIR}
)

set(INSTALL_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
    GROUP_READ GROUP_WRITE GROUP_WRITE
    WORLD_READ WORLD_WRITE WORLD_EXECUTE)


install(DIRECTORY config logs 
    DIRECTORY_PERMISSIONS ${INSTALL_PERMISSIONS} 
    DESTINATION ".") 
