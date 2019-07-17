
#include <gtest/gtest.h>

#include <llvm/IR/LLVMContext.h>
#include <routing/engine/clang_cc1_driver.h>
#include <routing/fmt.h>
#include <memory>
#include "llvm/Support/Error.h"
#include "routing/engine/SimpleOrcJit.h"

class Orc_test : public ::testing::Test
{
public:
    void SetUp() override
    {
        auto jit = SimpleOrcJit::create();

        ASSERT_TRUE(!!jit) << "Jit not created";

        m_jit = std::move(*jit);

        auto module = m_clang_driver.compile_source_code(
            "../resources/engine_orc_test_bin/simple_function.cpp", m_context);


        if (auto err = module.takeError())
        {
            llvm::report_fatal_error(std::move(err));
            FAIL();
        }

        auto error = m_jit->add(std::move(*module));

        ASSERT_FALSE(!!error) << "compilation not succeeded";
    }

protected:
    llvm::LLVMContext m_context;
    Clang_cc1_driver m_clang_driver;

    std::unique_ptr<SimpleOrcJit> m_jit;
};

extern "C" int external_unit_test_function(int value)
{
    return value * 2;
}

TEST_F(Orc_test, simple_function_lookup)
{
    auto symbol = m_jit->lookup("simple_function");

    if (auto err = symbol.takeError())
    {
        llvm::report_fatal_error(std::move(err));
    }

    int (*symbol_function)(int) = (int (*)(int))(symbol->getAddress());

    ASSERT_EQ(42, symbol_function(42));
}

TEST_F(Orc_test, external_function_lookup)
{
    auto symbol = m_jit->lookup("calling_external_function");

    if (auto err = symbol.takeError())
    {
        llvm::report_fatal_error(std::move(err));
    }

    int (*symbol_function)(int) = (int (*)(int))(symbol->getAddress());

    ASSERT_EQ(84, symbol_function(42));
}


