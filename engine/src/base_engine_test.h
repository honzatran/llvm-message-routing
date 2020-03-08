

#pragma once

#include <llvm/IR/LLVMContext.h>
#include <routing/engine/clang_cc1_driver.h>
#include <routing/fmt.h>
#include <memory>
#include <string_view>
#include "llvm/Support/Error.h"
#include "llvm/Support/raw_ostream.h"
#include "routing/engine/SimpleOrcJit.h"

#include <gtest/gtest.h>

namespace routing::engine
{
namespace detail
{
class Base_engine_common : public ::testing::Test
{
public:
    Base_engine_common(std::string_view file);

protected:
    std::string m_source_code_file;

    std::unique_ptr<SimpleOrcJit> m_jit;

    llvm::LLVMContext m_context;
    Clang_cc1_driver m_clang_driver;

    std::shared_ptr<routing::engine::File_jit_symbols> m_jit_symbols;

    void init_jit();
};
}  // namespace detail

/// Gets an error from the llvm::error
inline std::string
get_error_msg(llvm::Error error)
{
    std::string error_msg;
    llvm::raw_string_ostream stream(error_msg);

    llvm::handleAllErrors(
        std::move(error), [&stream](llvm::ErrorInfoBase& error_info) {
            error_info.log(stream);

            stream << "\n";
        });

    stream.flush();

    return error_msg;
}

/// Base class for engine tests
class Base_engine_function_test : public detail::Base_engine_common
{
public:
    Base_engine_function_test(std::string_view source_code_file)
        : detail::Base_engine_common(source_code_file)
    {
    }

    void SetUp() override;

protected:
};

class Base_engine_full_compile_test : public detail::Base_engine_common
{
public:
    Base_engine_full_compile_test(std::string_view source_code_file)
        : detail::Base_engine_common(source_code_file)
    {
    }

    void SetUp() override;

protected:
};
}  // namespace routing::engine
