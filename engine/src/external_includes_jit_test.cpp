
#include <gtest/gtest.h>

#include "base_engine_test.h"
#include "routing/engine/symbol_export_plugin.h"

#include <cstdint>

class External_include_tests : public routing::engine::Base_engine_function_test
{
public:
    External_include_tests()
        : routing::engine::Base_engine_function_test("external_includes.cpp")
    {
    }
};

TEST_F(External_include_tests, bits_utils)
{
    auto external_bits_util_symbol = m_jit_symbols->get_symbols("external_bits_util");

    ASSERT_EQ(1, external_bits_util_symbol.size());

    auto mangled_name = external_bits_util_symbol[0].get_mangled_symbol();
    llvm::StringRef symbol_name(&mangled_name[0], mangled_name.size());

    auto external_bits_function = m_jit->lookup(symbol_name);

    if (auto err = external_bits_function.takeError())
    {
        FAIL() << routing::engine::get_error_msg(std::move(err));
    }

    std::int64_t (*symbol_function)(int, int)
        = (int64_t(*)(int, int))(external_bits_function->getAddress());
}
