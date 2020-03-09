

#include "base_engine_test.h"

routing::engine::detail::Base_engine_common::Base_engine_common(
    std::string_view file)
    : m_source_code_file(file),
      m_jit_symbols(std::make_shared<routing::engine::File_jit_symbols>())
{
}

void
routing::engine::detail::Base_engine_common::init_jit()
{
    auto jit = SimpleOrcJit::create();

    ASSERT_TRUE(!!jit) << "Jit not created";

    m_jit = std::move(*jit);
}

void
routing::engine::Base_engine_function_test::SetUp()
{
    init_jit();

    auto module = m_clang_driver.compile_source_code(
        fmt::format("../resources/engine_orc_test_bin/{}", m_source_code_file),
        m_context,
        m_jit_symbols);

    if (auto err = module.takeError())
    {
        FAIL() << get_error_msg(std::move(err));
    }

    auto error = m_jit->add(std::move(*module));

    ASSERT_FALSE(!!error) << "compilation not succeeded";
}

void 
routing::engine::Base_engine_full_compile_test::SetUp()
{
    init_jit();

    auto path = m_clang_driver.tranform_source_code(
        fmt::format("../resources/engine_orc_test_bin/{}", m_source_code_file),
        m_context);

    if (auto err = path.takeError())
    {
        FAIL() << get_error_msg(std::move(err));
    }

    auto module = m_clang_driver.compile_source_code(
        *path,
        m_context,
        m_jit_symbols);

    if (auto err = module.takeError())
    {
        FAIL() << get_error_msg(std::move(err));
    }

    auto error = m_jit->add(std::move(*module));

    ASSERT_FALSE(!!error) << "compilation not succeeded";
}
