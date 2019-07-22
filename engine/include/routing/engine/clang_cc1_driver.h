#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/Error.h>

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <routing/engine/symbol_export_plugin.h>

#pragma once

class Clang_cc1_driver
{
public:
    Clang_cc1_driver() = default;

    // As long as the driver exists, source files remain on disk to allow
    // debugging JITed code.
    ~Clang_cc1_driver()
    {
        for (auto D : SoucreFileDeleters)
            D();
    }

    llvm::Expected<std::unique_ptr<llvm::Module>> compileTranslationUnit(
        std::string cppCode,
        llvm::LLVMContext &context);

    llvm::Expected<std::unique_ptr<llvm::Module>> compile_source_code(
        std::string const& source_code_path,
        llvm::LLVMContext &context,
        std::shared_ptr<routing::engine::File_jit_symbols> const& symbols);

private:
    std::vector<std::function<void()>> SoucreFileDeleters;
};
