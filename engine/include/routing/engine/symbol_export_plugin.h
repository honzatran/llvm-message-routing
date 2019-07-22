
#ifndef ROUTING_ENGINE_SYMBOL_EXPORT_PLUGIN_H
#define ROUTING_ENGINE_SYMBOL_EXPORT_PLUGIN_H

#include <clang/Frontend/FrontendPluginRegistry.h>
#include "absl/container/inlined_vector.h"
#include "absl/types/span.h"

#include <memory>

namespace routing
{
namespace engine
{
/// Single Jit symbol inside a cpp file, accessible for JIT lookup
class Jit_symbol
{
public:
    Jit_symbol(
        std::string const &symbol_name,
        std::string const &mangled_symbol)
        : m_symbol_name(symbol_name), m_mangled_symbol(mangled_symbol)
    {
    }

    std::string_view get_symbol_name() const { return m_symbol_name; }

    std::string_view get_mangled_symbol() const { return m_mangled_symbol; }

private:
    std::string m_symbol_name;
    std::string m_mangled_symbol;
};

/// Holds symbols inside a single cpp file, which are exported
class File_jit_symbols
{
public:
    using Find_symbol_result = absl::InlinedVector<Jit_symbol, 4>;

    /// Adds the symbol to the file
    auto add_symbol(Jit_symbol const &jit_symbol) -> void
    {
        m_symbols.push_back(jit_symbol);
    }

    /// Gets all symbols in the file
    auto get_symbols() const -> absl::Span<Jit_symbol const>
    {
        return absl::MakeSpan(m_symbols);
    }

    auto get_symbols(std::string_view symbol_name) const -> Find_symbol_result;

private:
    std::vector<Jit_symbol> m_symbols;
};

/// Clang plugin to extract the available symbols from the clang
class Symbol_export_plugin : public clang::PluginASTAction
{
public:
    /// Register a jit sybols for the given file
    static void register_jit_symbols(
        std::string const& file_name,
        std::shared_ptr<File_jit_symbols> const &jit_symbols);

    auto ParseArgs(
        const clang::CompilerInstance &CI,
        const std::vector<std::string> &arg) -> bool override
    {
        return true;
    }

    auto CreateASTConsumer(clang::CompilerInstance &CI, llvm::StringRef InFile)
        -> std::unique_ptr<clang::ASTConsumer> override;

    auto getActionType() -> ActionType override
    {
        return ActionType::AddAfterMainAction;
    }

private:
};

}  // namespace engine
}  // namespace routing

#endif
