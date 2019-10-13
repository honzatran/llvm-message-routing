

#include <algorithm>
#include "llvm/Support/Casting.h"
#include "llvm/Support/raw_ostream.h"
#include "routing/logger.h"
#include "routing/synchronized.h"
#include "routing/sys_util.h"

#include <routing/engine/symbol_export_plugin.h>

#include <clang/AST/ASTConsumer.h>
#include <clang/AST/Mangle.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Frontend/CompilerInstance.h>

#include <memory>
#include <unordered_map>

#include <routing/engine/annotation.h>

using namespace routing::engine;

routing::Synchronized<
    std::unordered_map<std::string, std::shared_ptr<File_jit_symbols>>>
    g_registered_file_symbols;

auto
File_jit_symbols::get_symbols(std::string_view symbol_name) const
    -> File_jit_symbols::Find_symbol_result
{
    absl::InlinedVector<Jit_symbol, 4> symbols;

    for (auto const& symbol : m_symbols)
    {
        if (symbol.get_symbol_name() == symbol_name)
        {
            symbols.push_back(symbol);
        }
    }

    return symbols;
}

bool
has_annotation(clang::Decl* decl, absl::string_view annotation_text)
{
    if (decl->hasAttr<clang::AnnotateAttr>())
    {
        auto* annotateAttribute = decl->getAttr<clang::AnnotateAttr>();

        llvm::StringRef tmp(annotation_text.begin(), annotation_text.length());

        if (annotateAttribute)
        {
            return annotateAttribute->getAnnotation() == tmp;
        }
    }

    return false;
}

class Symbol_visitor : public clang::RecursiveASTVisitor<Symbol_visitor>
{
public:
    Symbol_visitor(
        clang::MangleContext* context,
        File_jit_symbols* exported_symbols)
        : m_context(context), m_exported_symbols(exported_symbols)
    {
        m_logger = routing::get_default_logger("Symbol_exporter");
    }

    bool VisitFunctionDecl(clang::FunctionDecl* function_decl)
    {
        if (!has_annotation(function_decl, function_annotation()))
        {
            return true;
        }

        if (function_decl->isInStdNamespace()
            || function_decl->isCXXClassMember())
        {
            return true;
        }

        auto namespace_info = function_decl->getNameInfo();
        m_logger->info("Namespace {}", namespace_info.getName().getAsString());

        if (llvm::isa<clang::CXXConstructorDecl>(function_decl)
            || llvm::isa<clang::CXXDestructorDecl>(function_decl))
        {
            m_logger->info(
                "Visiting structor function {}",
                function_decl->getNameAsString());
            return true;
        }

        std::string mangled_name = mangle_name(function_decl);

        m_logger->info(
            "Visiting function {} mangled to {} ",
            function_decl->getNameAsString(),
            mangled_name);

        if (m_exported_symbols)
        {
            m_exported_symbols->add_symbol(
                Jit_symbol(function_decl->getNameAsString(), mangled_name));
        }

        return true;
    }

    bool VisitCXXRecordDecl(clang::CXXRecordDecl* class_record)
    {
        if (has_annotation(class_record, router_annotation()))
        {
            m_logger->info(
                "Annotated class {}", class_record->getNameAsString());

            m_exported_symbols->add_router_class(
                class_record->getNameAsString());
        }

        return true;
    }

    bool VisitCXXMethodDecl(clang::CXXMethodDecl* method_decl)
    {
        std::string class_name = method_decl->getParent()->getNameAsString();

        if (method_decl->isStatic()
            && has_annotation(method_decl, router_factory_annotation()))
        {
            std::string mangled_name = mangle_name(method_decl);

            m_logger->info(
                "class name {} Factory method {} mangled to {}",
                class_name,
                method_decl->getNameAsString(),
                mangled_name);

            m_exported_symbols->add_automaton_factory(
                class_name,
                Jit_symbol(
                    method_decl->getQualifiedNameAsString(), mangled_name));
        }
        else if (
            method_decl->isInstance()
            && has_annotation(method_decl, router_entrance_annotation()))
        {
            std::string mangled_name = mangle_name(method_decl);

            m_logger->info(
                "Class name {} Entrance method {} mangled to {}",
                class_name,
                method_decl->getNameAsString(),
                mangle_name(method_decl));

            m_exported_symbols->add_automaton_entrance(
                class_name,
                Jit_symbol(
                    method_decl->getQualifiedNameAsString(), mangled_name));
        }

        return true;
    }

private:
    clang::MangleContext* m_context;

    File_jit_symbols* m_exported_symbols;

    routing::Logger_t m_logger;

    std::string mangle_name(clang::NamedDecl* decl)
    {
        if (m_context->shouldMangleDeclName(decl))
        {
            std::string tmp;

            llvm::raw_string_ostream oss(tmp);
            m_context->mangleName(decl, oss);
            oss.flush();

            return oss.str();
        }

        return decl->getQualifiedNameAsString();
    }
};

class Symbol_exporter : public clang::ASTConsumer
{
public:
    Symbol_exporter(
        clang::CompilerInstance& compiler_instance,
        std::shared_ptr<File_jit_symbols> const& symbols_ptr)
        : m_compiler_instance(compiler_instance)
    {
        if (symbols_ptr)
        {
            m_symbols = symbols_ptr.get();
        }
    }

    void HandleTranslationUnit(clang::ASTContext& Ctx) override
    {
        auto mangle_context = std::unique_ptr<clang::ItaniumMangleContext>(
            clang::ItaniumMangleContext::create(
                Ctx, m_compiler_instance.getDiagnostics()));

        Symbol_visitor visitor(mangle_context.get(), m_symbols);

        visitor.TraverseDecl(Ctx.getTranslationUnitDecl());
    }

private:
    clang::CompilerInstance& m_compiler_instance;
    File_jit_symbols* m_symbols = nullptr;
};

void
Symbol_export_plugin::register_jit_symbols(
    std::string const& file_name,
    std::shared_ptr<File_jit_symbols> const& symbols)
{
    auto registered_file_symbols_lock = g_registered_file_symbols.lock();

    registered_file_symbols_lock->insert_or_assign(file_name, symbols);
}

std::unique_ptr<clang::ASTConsumer>
Symbol_export_plugin::CreateASTConsumer(
    clang::CompilerInstance& clang,
    llvm::StringRef inFile)
{
    auto logger = routing::get_default_logger("Symbol_exporter");

    std::shared_ptr<File_jit_symbols> ptr;

    {
        auto registered_symbol_file_lock = g_registered_file_symbols.lock();

        if (auto it = registered_symbol_file_lock->find(inFile.str());
            it != registered_symbol_file_lock->end())
        {
            ptr = it->second;
        }
    }

    logger->info("Processing file {}", inFile.str());
    return std::make_unique<Symbol_exporter>(clang, ptr);
}
