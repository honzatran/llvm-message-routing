

#include "base_engine_test.h"

extern "C" int
external_unit_test_function(int value)
{
    return value * 2;
}


using namespace routing::engine;

class Simple_function_test : public Base_engine_function_test
{
public:
    Simple_function_test() : Base_engine_function_test("simple_function.cpp") {}
};

TEST_F(Simple_function_test, no_function_exported)
{
    ASSERT_EQ(0, m_jit_symbols->get_symbols().size());
}

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
    auto test_cpp_function_symbols
        = m_jit_symbols->get_symbols("test_cpp_function");

    ASSERT_EQ(1, test_cpp_function_symbols.size());

    auto mangled_name = test_cpp_function_symbols[0].get_mangled_symbol();

    llvm::StringRef symbol_name(&mangled_name[0], mangled_name.size());

    auto symbol = m_jit->lookup(symbol_name);

    if (auto err = symbol.takeError())
    {
        // llvm::report_fatal_error(std::move(err));
        FAIL() << get_error_msg(std::move(err));
    }

    int (*symbol_function)(int, std::size_t)
        = (int (*)(int, std::size_t))(symbol->getAddress());

    ASSERT_EQ(128, symbol_function(42, 2));
}
