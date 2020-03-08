

#include <gtest/gtest.h>

#include <routing/engine/clang_cc1_driver.h>
#include "llvm/IR/LLVMContext.h"
#include "routing/engine/clang_cc1_driver.h"

#include "base_engine_test.h"
#include "routing/engine/symbol_export_plugin.h"

class Router_code_transform_test : public ::testing::Test
{
public:

    
protected:
    Clang_cc1_driver m_driver;
    llvm::LLVMContext m_context;
};

TEST_F(Router_code_transform_test, success_compilation)
{
    auto path = m_driver.tranform_source_code(
        "../resources/engine_orc_test_bin/router.cpp", m_context);

    if (auto err = path.takeError())
    {
        FAIL() << routing::engine::get_error_msg(std::move(err));
    }

    auto symbols = std::make_shared<routing::engine::File_jit_symbols>();

    auto module = m_driver.compile_source_code(*path, m_context, symbols);

    if (auto err = module.takeError())
    {
        FAIL() << routing::engine::get_error_msg(std::move(err));
    }
}
