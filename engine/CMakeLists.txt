
if(WIN32)
    set(OS_NAME "Win")
    set(DISABLE_RTTI_FLAG /GR-)
elseif(APPLE)
    set(OS_NAME "OSX")
    set(DISABLE_RTTI_FLAG -fno-rtti)
elseif(UNIX)
    set(OS_NAME "Linux")
    set(DISABLE_RTTI_FLAG -fno-rtti)
else()
    message(FATAL_ERROR "Operating system not supported")
endif()
message(STATUS "System: ${OS_NAME}")

list(APPEND ENGINE_HEADERS
    "include/routing/engine/SimpleOrcJit.h"
    "include/routing/engine/ClangCC1Driver.h"
    "include/routing/engine/ClangCC1Args_${OS_NAME}.h"
    "include/routing/engine/optimizer.h"
)

list (APPEND ENGINE_SRC
    # "src/JitFromScratch.cpp"
    "src/ClangCC1Driver.cpp"
    "src/SimpleOrcJit.cpp"
    "src/optimizer.cpp"
    ${ENGINE_HEADERS}
)

list (APPEND ENGINE_TEST_SRC
    "src/init_test.cpp"
    "src/orc_jit_test.cpp"
    ${ENGINE_HEADERS}
)

# add_executable(JitFromScratch
#     ${ENGINE_SRC}
# )
#
# set_target_properties(JitFromScratch PROPERTIES
#     CXX_STANDARD 17
#     CXX_STANDARD_REQUIRED ON
#     CXX_EXTENSIONS OFF
# )

# add LLVM libraries, set LLVM_* variables
find_package(LLVM 8.0 REQUIRED PATHS ${LLVM_DIR})
message(STATUS "LLVM_DIR: ${LLVM_DIR}")
message(STATUS "LLVM_PACKAGE_VERSION: ${LLVM_PACKAGE_VERSION}")

list(APPEND CMAKE_MODULE_PATH ${LLVM_DIR})
include(AddLLVM)

# add Clang libraries
include(${LLVM_BUILD_BINARY_DIR}/lib/cmake/clang/ClangTargets.cmake)
#
# if(NOT LLVM_ENABLE_RTTI)
#     target_compile_options(JitFromScratch PRIVATE ${DISABLE_RTTI_FLAG})
# endif()
#
# if(USE_LLD)
# 	message(STATUS "Add option to link with LLD")
# 	target_link_libraries(JitFromScratch PRIVATE "-fuse-ld=lld")
# endif()

# find Clang source directory
if(EXISTS ${LLVM_BUILD_MAIN_SRC_DIR}/tools/clang)
    set(CLANG_SRC_DIR_PREFIX tools) # sources in-tree (default build)
elseif(EXISTS ${LLVM_BUILD_MAIN_SRC_DIR}/../clang)
    set(CLANG_SRC_DIR_PREFIX ..) # sources out-of-tree (ENABLE_PROJECTS build) 
else()
    message(FATAL_ERROR "Cannot find Clang sources")
endif()

message(STATUS "LLVM Source Directory: ${LLVM_BUILD_MAIN_SRC_DIR}")
message(STATUS "Clang Source Directory: ${LLVM_BUILD_MAIN_SRC_DIR}/${CLANG_SRC_DIR_PREFIX}/clang")
message(STATUS "LLVM/Clang Build Directory: ${LLVM_BUILD_BINARY_DIR}")

# LLVM definitions
separate_arguments(LLVM_DEFINITIONS)
# target_compile_definitions(JitFromScratch PRIVATE
#     ${LLVM_DEFINITIONS}
# )

# JitFromScratch dependencies
llvm_map_components_to_libnames(LLVM_LIBS
    core
    executionengine
    native
    object
    orcjit
    runtimedyld
    support
)

# target_link_libraries(JitFromScratch PRIVATE
#     ${LLVM_LIBS}
# )

# Project-specific definitions
# target_compile_definitions(JitFromScratch
#   PRIVATE
#     # Supply path to Clang resources in the LLVM/Clang build directory
#     JIT_FROM_SCRATCH_CLANG_RESOURCE_DIR=${LLVM_BUILD_BINARY_DIR}/lib/clang/${LLVM_PACKAGE_VERSION}
# )

# target_compile_definitions(JitFromScratch PRIVATE ${LLVM_DEFINITIONS})

# Clang dependencies

routing_library(
    TARGET 
        "routing_orc"
    EXPORT_NAME
        "orc"
    SOURCES
        ${ENGINE_SRC}
    PUBLIC_LIBRARIES
        ${LLVM_LIBS}
        LLVMX86CodeGen
        LLVMX86AsmPrinter
        LLVMX86AsmParser
        LLVMX86Desc
        LLVMX86Info
        LLVMX86Disassembler
        LLVMAnalysis
        LLVMCodeGen
        LLVMCore
        LLVMipo
        LLVMInstCombine
        LLVMInstrumentation
        LLVMMC
        LLVMMCParser
        LLVMObjCARCOpts
        LLVMOption
        LLVMScalarOpts
        LLVMSupport
        LLVMTransformUtils
        LLVMVectorize
        clangBasic
        clangCodeGen
        clangDriver
        clangFrontend
        clangFrontendTool
        LLVMAsmPrinter
        LLVMDebugInfoCodeView
        LLVMDebugInfoMSF
        LLVMGlobalISel
        LLVMSelectionDAG
        LLVMX86Utils
        LLVMMCDisassembler
        LLVMCoroutines
        LLVMCoverage
        LLVMLTO
        LLVMPasses
        LLVMTarget
        LLVMBitWriter
        LLVMIRReader
        LLVMAsmParser
        LLVMLinker
        LLVMObject
        clangRewriteFrontend
        clangARCMigrate
        clangStaticAnalyzerFrontend
        LLVMProfileData
        clangParse
        clangSerialization
        LLVMBitReader
        clangSema
        clangEdit
        clangStaticAnalyzerCheckers
        clangASTMatchers
        clangStaticAnalyzerCore
        clangAnalysis
        clangAST
        clangRewrite
        clangLex
        LLVMDemangle
        routing::common
    PUBLIC_INCLUDE_DIRS
        "include"
        ${LLVM_INCLUDE_DIRS}
        ${LLVM_BUILD_BINARY_DIR}/tools/clang/include
        ${LLVM_BUILD_MAIN_SRC_DIR}/${CLANG_SRC_DIR_PREFIX}/clang/include
    PRIVATE_INCLUDE_DIRS
        "src"
    PRIVATE_COMPILE_FLAGS
        ${DISABLE_RTTI_FLAG}
    PRIVATE_COMPILE_DEFINITIONS
        ${LLVM_DEFINITIONS}
        JIT_FROM_SCRATCH_CLANG_RESOURCE_DIR=${LLVM_BUILD_BINARY_DIR}/lib/clang/${LLVM_PACKAGE_VERSION}
)

routing_exe(
    TARGET 
        "engine_orc"
    SOURCES
        "src/JitFromScratch.cpp"
    PUBLIC_LIBRARIES
        routing::orc
    PRIVATE_COMPILE_FLAGS
        ${DISABLE_RTTI_FLAG}
)

routing_test(
    TARGET
        "engine_orc_test"
    SOURCES
        ${ENGINE_TEST_SRC}
    PUBLIC_LIBRARIES
        routing::orc
    PRIVATE_COMPILE_FLAGS
        ${DISABLE_RTTI_FLAG}
)






# get_target_property(ALL_INCLUDE_DIRECTORIES JitFromScratch INCLUDE_DIRECTORIES)
# message(STATUS "Include directories: ${ALL_INCLUDE_DIRECTORIES}")
#
# get_target_property(ALL_LINK_LIBRARIES "routing_orc" LINK_LIBRARIES)
# message(STATUS "Link libraries: ${ALL_LINK_LIBRARIES}")
# 
# get_target_property(ALL_COMPILE_OPTIONS "routing_orc" COMPILE_OPTIONS)
# message(STATUS "Compile options: ${ALL_COMPILE_OPTIONS}")
#
# get_target_property(ALL_COMPILE_DEFINITIONS "routing_orc" COMPILE_DEFINITIONS)
# message(STATUS "Compile definitions: ${ALL_COMPILE_DEFINITIONS}")
#
# message(STATUS "Other flags: ${CMAKE_CXX_FLAGS}")
# message(STATUS "Other flags Debug: ${CMAKE_CXX_FLAGS_DEBUG}")
# message(STATUS "Other flags Release: ${CMAKE_CXX_FLAGS_RELEASE}")
