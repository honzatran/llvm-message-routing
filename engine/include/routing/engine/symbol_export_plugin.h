
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
    Jit_symbol() = default;

    Jit_symbol(
        std::string const& symbol_name,
        std::string const& mangled_symbol)
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
    void add_symbol(Jit_symbol const& jit_symbol)
    {
        m_symbols.push_back(jit_symbol);
    }

    /// Gets all symbols in the file
    auto get_symbols() const -> absl::Span<Jit_symbol const>
    {
        return absl::MakeSpan(m_symbols);
    }

    auto get_symbols(std::string_view symbol_name) const -> Find_symbol_result;

    void add_router_class(std::string const& class_name)
    {
        m_router_classes.push_back(class_name);
    }

    void add_automaton_factory(
        std::string const& class_name,
        Jit_symbol const& jit_symbol)
    {
        m_factory_symbols[class_name] = jit_symbol;
    }

    void add_automaton_entrance(
        std::string const& class_name,
        Jit_symbol const& jit_symbol)
    {
        m_entrance_symbols[class_name] = jit_symbol;
    }

    bool contains_router(std::string const& class_name)
    {
        return std::find(
                   m_router_classes.begin(), m_router_classes.end(), class_name)
               != m_router_classes.end();
    }

    std::optional<Jit_symbol> get_factory(std::string const& router_name) const
    {
        auto it = m_factory_symbols.find(router_name);

        if (it != m_factory_symbols.end())
        {
            return {it->second};
        }

        return {};
    }

    std::optional<Jit_symbol> get_entrance(std::string const& router_name) const
    {
        auto it = m_entrance_symbols.find(router_name);

        if (it != m_entrance_symbols.end())
        {
            return {it->second};
        }

        return {};
    }

private:
    std::vector<Jit_symbol> m_symbols;

    std::vector<std::string> m_router_classes;

    std::unordered_map<std::string, Jit_symbol> m_factory_symbols;
    std::unordered_map<std::string, Jit_symbol> m_entrance_symbols;
};

/// Clang plugin to extract the available symbols from the clang
class Symbol_export_plugin : public clang::PluginASTAction
{
public:
    /// Register a jit sybols for the given file
    static void register_jit_symbols(
        std::string const& file_name,
        std::shared_ptr<File_jit_symbols> const& jit_symbols);

    /// Register a jit sybols for the given file
    static std::unique_ptr<llvm::Module> get_generated_module(
        std::string const& file_name);

    auto ParseArgs(
        const clang::CompilerInstance& CI,
        const std::vector<std::string>& arg) -> bool override
    {
        return true;
    }

    auto CreateASTConsumer(clang::CompilerInstance& CI, llvm::StringRef InFile)
        -> std::unique_ptr<clang::ASTConsumer> override;

    auto getActionType() -> ActionType override
    {
        return ActionType::AddBeforeMainAction;
    }

private:
};

}  // namespace engine
}  // namespace routing

#endif
