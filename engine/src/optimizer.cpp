

#include <routing/engine/optimizer.h>

#include <llvm/ExecutionEngine/Orc/Core.h>
#include <llvm/ExecutionEngine/Orc/ThreadSafeModule.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Support/Error.h>
#include <llvm/IR/PassManager.h>
#include "routing/logger.h"
#include "routing/sys_util.h"

llvm::Expected<llvm::orc::ThreadSafeModule>
routing::engine::LegacyPassManagerOptimizer::operator()(
    llvm::orc::ThreadSafeModule thread_safe_module,
    llvm::orc::MaterializationResponsibility const& responsibility)
{
    llvm::Module& module = *thread_safe_module.getModule();

    llvm::legacy::FunctionPassManager function_pass_manager(&module);

    m_builder.populateFunctionPassManager(function_pass_manager);

    function_pass_manager.doInitialization();

    for (llvm::Function& f : module)
    {
        function_pass_manager.run(f);
    }

    function_pass_manager.doFinalization();

    llvm::legacy::PassManager pass_manager;
    m_builder.populateModulePassManager(pass_manager);

    return std::move(thread_safe_module);
}
