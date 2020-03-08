

#include <algorithm>

#include <clang/AST/Decl.h>
#include <clang/Rewrite/Core/Rewriter.h>
#include "absl/types/span.h"
#include "clang/AST/GlobalDecl.h"
#include "clang/Basic/LangOptions.h"
#include "clang/Basic/Specifiers.h"
#include "clang/Basic/TypeTraits.h"
#include "clang/Sema/Ownership.h"

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/GlobalValue.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/raw_ostream.h"

#include "routing/fmt.h"
#include "routing/logger.h"
#include "routing/synchronized.h"
#include "routing/sys_util.h"
#include "spdlog/fmt/bundled/format.h"

#include <routing/engine/symbol_export_plugin.h>
#include <routing/file_util.h>

#include <../lib/CodeGen/CGCXXABI.h>
#include <../lib/CodeGen/CodeGenFunction.h>
#include <../lib/CodeGen/CodeGenModule.h>
#include <clang/AST/ASTConsumer.h>
#include <clang/AST/Mangle.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/CodeGen/ModuleBuilder.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Sema/Sema.h>

#include <iterator>
#include <memory>
#include <unordered_map>

#include <routing/engine/annotation.h>

using namespace routing::engine;

routing::Synchronized<
    std::unordered_map<std::string, std::shared_ptr<File_jit_symbols>>>
    g_registered_file_symbols;

llvm::LLVMContext g_context;

routing::Synchronized<
    std::unordered_map<std::string, std::unique_ptr<llvm::Module>>>
    g_generated_modules;

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
        clang::Sema& sema,
        File_jit_symbols* exported_symbols,
        clang::CodeGenerator* code_generator,
        clang::TranslationUnitDecl* translation_unit_decl)
        : m_context(context),
          m_sema(sema),
          m_translation_unit_decl(translation_unit_decl),
          m_exported_symbols(exported_symbols),
          m_code_generator(code_generator)
    {
        m_logger = routing::get_default_logger("Symbol_exporter");
    }

    bool VisitFunctionDecl(clang::FunctionDecl* function_decl)
    {
        bool is_engine_function
            = has_annotation(function_decl, function_annotation());

        bool is_engine_factory_function
            = has_annotation(function_decl, router_factory_annotation());

        if (!is_engine_function && !is_engine_factory_function)
        {
            return true;
        }

        if (function_decl->isInStdNamespace()
            || function_decl->isCXXClassMember())
        {
            return true;
        }

        auto namespace_info = function_decl->getNameInfo();
        m_logger->debug("Namespace {}", namespace_info.getName().getAsString());

        if (llvm::isa<clang::CXXConstructorDecl>(function_decl)
            || llvm::isa<clang::CXXDestructorDecl>(function_decl))
        {
            m_logger->debug(
                "Visiting structor function {}",
                function_decl->getNameAsString());
            return true;
        }

        std::string mangled_name = mangle_name(function_decl);

        m_logger->debug(
            "Visiting function {} mangled to {} ",
            function_decl->getNameAsString(),
            mangled_name);

        if (m_exported_symbols)
        {
            if (is_engine_function)
            {
                m_exported_symbols->add_symbol(
                    Jit_symbol(function_decl->getNameAsString(), mangled_name));
            }
            else
            {
                m_exported_symbols->add_automaton_factory(
                    "Automaton",
                    Jit_symbol(function_decl->getNameAsString(), mangled_name));
            }
        }

        return true;
    }

    bool VisitCXXRecordDecl(clang::CXXRecordDecl* class_record)
    {
        if (has_annotation(class_record, router_annotation()))
        {
            m_logger->debug(
                "Annotated class {}", class_record->getNameAsString());

            m_exported_symbols->add_router_class(
                class_record->getNameAsString());

            auto& identifier = m_context->getASTContext().Idents.getOwn(
                "G_GENERATED_GLOBAL");

            auto qual_type = clang::QualType(
                class_record->getTypeForDecl(), clang::Qualifiers::Const);

            m_logger->debug("IDENTIFIER {}", identifier.getName().str());
            auto& codegen_module = m_code_generator->CGM();

            clang::VarDecl* var_decl = clang::VarDecl::Create(
                class_record->getASTContext(),
                m_translation_unit_decl,
                clang::SourceLocation(),
                clang::SourceLocation(),
                &identifier,
                qual_type,
                class_record->getASTContext().getTrivialTypeSourceInfo(
                    qual_type),
                clang::StorageClass::SC_Extern);

            clang::GlobalDecl global_decl(var_decl);

            clang::DeclGroupRef decl_group_ref(var_decl);

            if (var_decl->getDeclContext()->getParent())
            {
                m_logger->debug("has parent");
            }
            else
            {
                m_logger->debug("no parent");
            }

            m_code_generator->HandleTopLevelDecl(decl_group_ref);
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

            m_logger->debug(
                "class name {} Factory method {} mangled to {}",
                class_name,
                method_decl->getNameAsString(),
                mangled_name);

            // m_exported_symbols->add_automaton_factory(
            //     class_name,
            //     Jit_symbol(
            //         method_decl->getQualifiedNameAsString(), mangled_name));
        }
        else if (
            method_decl->isInstance()
            && has_annotation(method_decl, router_entrance_annotation()))
        {
            std::string mangled_name = mangle_name(method_decl);

            m_logger->debug(
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
    clang::Sema& m_sema;
    clang::TranslationUnitDecl* m_translation_unit_decl;

    File_jit_symbols* m_exported_symbols;

    clang::CodeGenerator* m_code_generator;

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

    void create_init_function(
        clang::CXXRecordDecl* cxx_record,
        clang::VarDecl* decl,
        llvm::GlobalVariable* global_var)
    {
        auto& CGM = m_code_generator->CGM();

        llvm::FunctionType* fn_type
            = llvm::FunctionType::get(CGM.VoidTy, false);

        auto default_constructor = get_default_constructor(cxx_record);

        llvm::SmallString<256> fn_name;
        {
            llvm::raw_svector_ostream stream_name(fn_name);
            CGM.getCXXABI().getMangleContext().mangleCXXCtor(
                default_constructor,
                clang::CXXCtorType::Ctor_Complete,
                stream_name);
        }

        clang::CodeGen::CGFunctionInfo const& function_info
            = CGM.getTypes().arrangeNullaryFunction();

        auto fn = CGM.CreateGlobalInitOrDestructFunction(
            fn_type, fn_name.str(), function_info);

        clang::CodeGen::CodeGenFunction CGF(CGM);

        CGF.GenerateCXXGlobalVarDeclInitFunc(fn, decl, global_var, true);
    }

    static clang::CXXConstructorDecl* get_default_constructor(
        clang::CXXRecordDecl* cxx_record)
    {
        for (auto constructor : cxx_record->ctors())
        {
            if (constructor->isDefaultConstructor())
            {
                return constructor;
            }
        }

        return nullptr;
    }
};

class Symbol_exporter : public clang::ASTConsumer
{
public:
    Symbol_exporter(
        clang::CompilerInstance& compiler_instance,
        std::shared_ptr<File_jit_symbols> const& symbols_ptr,
        std::unique_ptr<clang::CodeGenerator> code_generator)
        : m_compiler_instance(compiler_instance),
          m_code_generator(std::move(code_generator))
    {
        if (symbols_ptr)
        {
            m_symbols = symbols_ptr.get();
        }

        m_logger = routing::get_default_logger("Symbol_exporter");
    }

    /// Initialize - This is called to initialize the consumer, providing the
    /// ASTContext.
    void Initialize(clang::ASTContext& Context) override
    {
        m_code_generator->Initialize(Context);

        m_compiler_instance.createSema(
            clang::TranslationUnitKind::TU_Complete, nullptr);
    }

    /// HandleTopLevelDecl - Handle the specified top-level declaration.  This
    /// is called by the parser to process every top-level Decl*.
    ///
    /// \returns true to continue parsing, or false to abort parsing.
    bool HandleTopLevelDecl(clang::DeclGroupRef D) override
    {
        return m_code_generator->HandleTopLevelDecl(D);
    }

    /// This callback is invoked each time an inline (method or friend)
    /// function definition in a class is completed.
    void HandleInlineFunctionDefinition(clang::FunctionDecl* D) override
    {
        m_code_generator->HandleInlineFunctionDefinition(D);
    }

    /// HandleInterestingDecl - Handle the specified interesting declaration.
    /// This is called by the AST reader when deserializing things that might
    /// interest the consumer. The default implementation forwards to
    /// HandleTopLevelDecl.
    void HandleInterestingDecl(clang::DeclGroupRef D) override
    {
        m_code_generator->HandleInterestingDecl(D);
    }

    /// HandleTranslationUnit - This method is called when the ASTs for entire
    /// translation unit have been parsed.
    void HandleTranslationUnit(clang::ASTContext& Ctx) override
    {
        m_code_generator->HandleTranslationUnit(Ctx);

        auto mangle_context = std::unique_ptr<clang::ItaniumMangleContext>(
            clang::ItaniumMangleContext::create(
                Ctx, m_compiler_instance.getDiagnostics()));

        Symbol_visitor visitor(
            mangle_context.get(),
            m_compiler_instance.getSema(),
            m_symbols,
            m_code_generator.get(),
            Ctx.getTranslationUnitDecl());

        visitor.TraverseDecl(Ctx.getTranslationUnitDecl());

        std::unique_ptr<llvm::Module> generated_module(
            m_code_generator->ReleaseModule());

        auto locked_modules = g_generated_modules.lock();
        auto& module_ptr = (*locked_modules)[generated_module->getName().str()];

        module_ptr = std::move(generated_module);
    }

    /// HandleTagDeclDefinition - This callback is invoked each time a TagDecl
    /// (e.g. struct, union, enum, class) is completed.  This allows the client
    /// to hack on the type, which can occur at any point in the file (because
    /// these can be defined in declspecs).
    void HandleTagDeclDefinition(clang::TagDecl* D) override
    {
        m_code_generator->HandleTagDeclDefinition(D);
    }

    /// This callback is invoked the first time each TagDecl is required to
    /// be complete.
    void HandleTagDeclRequiredDefinition(const clang::TagDecl* D) override
    {
        m_code_generator->HandleTagDeclRequiredDefinition(D);
    }

    /// Invoked when a function is implicitly instantiated.
    /// Note that at this point point it does not have a body, its body is
    /// instantiated at the end of the translation unit and passed to
    /// HandleTopLevelDecl.
    void HandleCXXImplicitFunctionInstantiation(clang::FunctionDecl* D) override
    {
        m_code_generator->HandleCXXImplicitFunctionInstantiation(D);
    }

    /// Handle the specified top-level declaration that occurred inside
    /// and ObjC container.
    /// The default implementation ignored them.
    void HandleTopLevelDeclInObjCContainer(clang::DeclGroupRef D) override
    {
        m_code_generator->HandleTopLevelDeclInObjCContainer(D);
    }

    /// Handle an ImportDecl that was implicitly created due to an
    /// inclusion directive.
    /// The default implementation passes it to HandleTopLevelDecl.
    void HandleImplicitImportDecl(clang::ImportDecl* D) override
    {
        m_code_generator->HandleImplicitImportDecl(D);
    }

    /// CompleteTentativeDefinition - Callback invoked at the end of a
    /// translation unit to notify the consumer that the given tentative
    /// definition should be completed.
    ///
    /// The variable declaration itself will be a tentative
    /// definition. If it had an incomplete array type, its type will
    /// have already been changed to an array of size 1. However, the
    /// declaration remains a tentative definition and has not been
    /// modified by the introduction of an implicit zero initializer.
    void CompleteTentativeDefinition(clang::VarDecl* D) override
    {
        m_code_generator->CompleteTentativeDefinition(D);
    }

    /// Callback invoked when an MSInheritanceAttr has been attached to a
    /// CXXRecordDecl.
    void AssignInheritanceModel(clang::CXXRecordDecl* RD) override
    {
        m_code_generator->AssignInheritanceModel(RD);
    }

    /// HandleCXXStaticMemberVarInstantiation - Tell the consumer that this
    // variable has been instantiated.
    void HandleCXXStaticMemberVarInstantiation(clang::VarDecl* D) override
    {
        m_code_generator->HandleCXXStaticMemberVarInstantiation(D);
    }

    /// Callback involved at the end of a translation unit to
    /// notify the consumer that a vtable for the given C++ class is
    /// required.
    ///
    /// \param RD The class whose vtable was used.
    void HandleVTable(clang::CXXRecordDecl* RD) override
    {
        m_code_generator->HandleVTable(RD);
    }

private:
    routing::Logger_t m_logger;
    clang::CompilerInstance& m_compiler_instance;
    File_jit_symbols* m_symbols = nullptr;

    std::unique_ptr<clang::CodeGenerator> m_code_generator;
};

void
Symbol_export_plugin::register_jit_symbols(
    std::string const& file_name,
    std::shared_ptr<File_jit_symbols> const& symbols)
{
    auto registered_file_symbols_lock = g_registered_file_symbols.lock();

    registered_file_symbols_lock->insert_or_assign(file_name, symbols);
}

std::unique_ptr<llvm::Module>
Symbol_export_plugin::get_generated_module(std::string const& file_name)
{
    auto module_lock_guard = g_generated_modules.lock();

    auto it = module_lock_guard->find(file_name);

    if (it == module_lock_guard->end())
    {
        return std::unique_ptr<llvm::Module>();
    }

    auto module = std::move(it->second);
    module_lock_guard->erase(it);

    return module;
}

class Automaton_method_info
{
public:
    Automaton_method_info(
        std::string const& method_name,
        std::string const& return_type)
        : m_method_name(method_name), m_return_type(return_type)
    {
    }

    /// Gets the method name
    absl::string_view get_method_name() const { return m_method_name; }

    /// Gets the method return type
    absl::string_view get_return_type() const { return m_return_type; }

    /// Adds a new parameter to the method info
    void add_parameter(
        std::string const& name,
        std::string const& qualified_type)
    {
        m_parameters_type[name] = qualified_type;
    }

    /// Gets mapping between the parameter name and it's full qualified name
    std::map<std::string, std::string> const& get_parameters() const
    {
        return m_parameters_type;
    }

private:
    // name of the method
    std::string m_method_name;

    // return type of the method
    std::string m_return_type;

    // mapping between the parameter name and it types
    std::map<std::string, std::string> m_parameters_type;
};

/// Holds data about the automaton class info
class Automaton_class_info
{
public:
    Automaton_class_info(std::string const& name) : m_name(name) {}

    /// Adds a new method to info class
    void add_entrance_method(Automaton_method_info const& method)
    {
        m_entrance_methods.push_back(method);
    }

    /// Marks the automaton as being generated
    void set_generate_as_global() { m_generate_global = true; }

    bool generate_as_global() const { return m_generate_global; }

    absl::string_view get_name() const { return absl::string_view(m_name); }

    absl::Span<Automaton_method_info const> get_entrance_methods() const
    {
        return absl::MakeSpan(m_entrance_methods);
    }

private:
    // qualified class name
    std::string m_name;

    // all entrance methods, of the class
    absl::InlinedVector<Automaton_method_info, 4> m_entrance_methods;

    // whether a global variable for this class should be generated.
    bool m_generate_global = false;
};

///
/// The visitor going through the AST and using the rewriter to change the code.
class Message_routing_transformer_visitor
    : public clang::RecursiveASTVisitor<Message_routing_transformer_visitor>
{
public:
    Message_routing_transformer_visitor()
    {
        m_logger = routing::get_default_logger("Message_routing_transfomer");
    }

    bool VisitCXXRecordDecl(clang::CXXRecordDecl* class_record)
    {
        if (has_annotation(class_record, router_annotation())
            && class_record->isCompleteDefinition())
        {
            auto it = m_automaton_classes.find(
                class_record->getQualifiedNameAsString());

            if (it != m_automaton_classes.end())
            {
                it->second.set_generate_as_global();
            }
            else
            {
                std::string qualified_name
                    = class_record->getQualifiedNameAsString();

                Automaton_class_info class_info(qualified_name);
                class_info.set_generate_as_global();

                m_automaton_classes.insert(
                    std::pair(qualified_name, class_info));
            }
        }

        return true;
    }

    bool VisitCXXMethodDecl(clang::CXXMethodDecl* method_decl)
    {
        std::string class_name
            = method_decl->getParent()->getQualifiedNameAsString();

        if (method_decl->isInstance()
            && has_annotation(method_decl, router_entrance_annotation()))
        {
            m_automaton_classes.insert(
                std::pair(class_name, method_decl->getNameAsString()));

            std::string return_type
                = method_decl->getReturnType().getAsString();

            Automaton_method_info method_info(
                method_decl->getNameAsString(), return_type);

            for (std::size_t i = 0; i < method_decl->getNumParams(); i++)
            {
                clang::ParmVarDecl* param_decl = method_decl->getParamDecl(i);
                clang::QualType qualType       = param_decl->getType();

                method_info.add_parameter(
                    param_decl->getQualifiedNameAsString(),
                    qualType.getAsString());
            }

            auto it = m_automaton_classes.find(class_name);

            if (it != m_automaton_classes.end())
            {
                it->second.add_entrance_method(method_info);
            }
            else
            {
                Automaton_class_info class_info(class_name);

                m_automaton_classes.insert(std::pair(class_name, class_info));
            }
        }

        return true;
    }

    std::map<std::string, Automaton_class_info> const& get_classes_to_generate()
    {
        return m_automaton_classes;
    }

private:
    routing::Logger_t m_logger;

    absl::InlinedVector<std::string, 4> m_globals_types;

    std::map<std::string, Automaton_class_info> m_automaton_classes;
};

///
/// Transform the cpp source code with regard to the annotations
class Message_routing_transformer : public clang::ASTConsumer
{
public:
    Message_routing_transformer(
        clang::CompilerInstance& compiler_instance,
        llvm::StringRef in_file)
        : m_in_file(in_file),
          m_logger(routing::get_default_logger("Transformer"))
    {
    }

    void HandleTranslationUnit(clang::ASTContext& Ctx) override
    {
        Message_routing_transformer_visitor visitor;

        auto* translation_unit = Ctx.getTranslationUnitDecl();

        visitor.TraverseDecl(translation_unit);

        std::string generated_code
            = generate_code(visitor.get_classes_to_generate());

        m_logger->info("Generated code {}", generated_code);

        routing::Text_file_writer writer(
            routing::replace_extension(m_in_file, "cpp"), routing::Write_tag{});

        writer.append_line(generated_code);
    }

private:
    std::string m_in_file;
    routing::Logger_t m_logger;

    std::string generate_code(
        std::map<std::string, Automaton_class_info> const& generated_classes)
    {
        std::string output = "\n";

        fmt::memory_buffer buffer;

        fmt::format_to(
            buffer,
            "#include \"{}\"\n",
            llvm::sys::path::filename(m_in_file).str());

        for (auto& [name, automaton_info] : generated_classes)
        {
            m_logger->info("Automaton name {}", name);

            if (automaton_info.generate_as_global())
            {
                fmt::format_to(
                    buffer,
                    "{0} g_generated_{0};\n",
                    automaton_info.get_name());

                append_methods(
                    name, automaton_info.get_entrance_methods(), buffer);
            }
        }

        return output.append(buffer.data(), buffer.size());
    }

    static void append_methods(
        absl::string_view type,
        absl::Span<Automaton_method_info const> methods,
        fmt::memory_buffer& buffer)
    {
        for (auto& method_info : methods)
        {
            fmt::format_to(
                buffer,
                "{} {}(",
                method_info.get_return_type(),
                method_info.get_method_name());

            bool has_param = false;

            for (auto const& [name, qualified_type] :
                 method_info.get_parameters())
            {
                has_param = true;
                fmt::format_to(buffer, "{} {},", qualified_type, name);
            }

            if (has_param)
            {
                // remove the last comma
                buffer.resize(buffer.size() - 1);
            }
            //
            fmt::format_to(buffer, ")\n{{");

            fmt::format_to(
                buffer,
                "g_generated_{}.{}(",
                type,
                method_info.get_method_name());

            for (auto const& [name, qualified_type] :
                 method_info.get_parameters())
            {
                fmt::format_to(buffer, "{},", name);
            }

            if (has_param)
            {
                // remove the last comma
                buffer.resize(buffer.size() - 1);
            }

            fmt::format_to(buffer, ");}}\n");
        }
    }
};

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

    if (ptr)
    {
        std::unique_ptr<clang::CodeGenerator> generator(
            clang::CreateLLVMCodeGen(
                clang.getDiagnostics(),
                inFile,
                clang.getHeaderSearchOpts(),
                clang.getPreprocessorOpts(),
                clang.getCodeGenOpts(),
                g_context,
                nullptr));

        logger->info("Processing file {}", inFile.str());

        return std::make_unique<Symbol_exporter>(
            clang, ptr, std::move(generator));
    }
    else
    {
        return std::make_unique<Message_routing_transformer>(clang, inFile);
    }
}
