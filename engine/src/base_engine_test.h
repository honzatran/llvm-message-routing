

#pragma once

#include <llvm/IR/LLVMContext.h>
#include <routing/engine/clang_cc1_driver.h>
#include <routing/fmt.h>
#include <memory>
#include "llvm/Support/Error.h"
#include "llvm/Support/raw_ostream.h"
#include "routing/engine/SimpleOrcJit.h"

#include <gtest/gtest.h>

namespace routing::engine 
{

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
class Base_engine_function_test : public ::testing::Test
{
public:
    Base_engine_function_test(std::string_view source_code_file)
        : m_source_code_file(source_code_file)
    {
    }

    void SetUp() override
    {
        auto jit = SimpleOrcJit::create();

        ASSERT_TRUE(!!jit) << "Jit not created";

        m_jit = std::move(*jit);

        m_jit_symbols = std::make_shared<routing::engine::File_jit_symbols>();

        auto module = m_clang_driver.compile_source_code(
            fmt::format(
                "../resources/engine_orc_test_bin/{}", m_source_code_file),
            m_context,
            m_jit_symbols);

        if (auto err = module.takeError())
        {
            llvm::report_fatal_error(std::move(err));
            FAIL();
        }

        auto error = m_jit->add(std::move(*module));

        ASSERT_FALSE(!!error) << "compilation not succeeded";
    }

protected:
    std::string m_source_code_file;

    llvm::LLVMContext m_context;
    Clang_cc1_driver m_clang_driver;

    std::unique_ptr<SimpleOrcJit> m_jit;

    std::shared_ptr<routing::engine::File_jit_symbols> m_jit_symbols;
};
}
