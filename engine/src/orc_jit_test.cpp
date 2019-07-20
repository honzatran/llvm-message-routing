
#include <gtest/gtest.h>

#include <llvm/IR/LLVMContext.h>
#include <routing/engine/clang_cc1_driver.h>
#include <routing/fmt.h>
#include <memory>
#include "llvm/Support/Error.h"
#include "llvm/Support/raw_ostream.h"
#include "routing/engine/SimpleOrcJit.h"

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

        auto module = m_clang_driver.compile_source_code(
            fmt::format(
                "../resources/engine_orc_test_bin/{}", m_source_code_file),
            m_context);

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
};

extern "C" int
external_unit_test_function(int value)
{
    return value * 2;
}

std::string
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

class Simple_function_test : public Base_engine_function_test
{
public:
    Simple_function_test() : Base_engine_function_test("simple_function.cpp") {}
};

TEST_F(Simple_function_test, simple_function_lookup)
{
    auto symbol = m_jit->lookup("simple_function");

    if (auto err = symbol.takeError())
    {
        FAIL() << get_error_msg(std::move(err));
    }

    int (*symbol_function)(int) = (int (*)(int))(symbol->getAddress());

    ASSERT_EQ(42, symbol_function(42));
}

TEST_F(Simple_function_test, external_function_lookup)
{
    auto symbol = m_jit->lookup("calling_external_function");

    if (auto err = symbol.takeError())
    {
        FAIL() << get_error_msg(std::move(err));
    }

    int (*symbol_function)(int) = (int (*)(int))(symbol->getAddress());

    ASSERT_EQ(84, symbol_function(42));
}

class Cpp_function_test : public Base_engine_function_test
{
public:
    Cpp_function_test() : Base_engine_function_test("cpp_functions.cpp") {}
};

TEST_F(Cpp_function_test, stl_function)
{
    auto symbol = m_jit->lookup("test_cpp_function");

    if (auto err = symbol.takeError())
    {
        // llvm::report_fatal_error(std::move(err));
        FAIL() << get_error_msg(std::move(err));
    }

    int (*symbol_function)(int, std::size_t)
        = (int (*)(int, std::size_t))(symbol->getAddress());

    ASSERT_EQ(128, symbol_function(42, 2));
}
