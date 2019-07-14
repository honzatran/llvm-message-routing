
#ifndef ROUTING_ENGINE_OPTIMIZER_H
#define ROUTING_ENGINE_OPTIMIZER_H

#include <llvm/Transforms/IPO/PassManagerBuilder.h>
#include "llvm/ExecutionEngine/Orc/Core.h"
#include "llvm/ExecutionEngine/Orc/ThreadSafeModule.h"
#include "llvm/Support/Error.h"

namespace routing
{
namespace engine
{
class LegacyPassManagerOptimizer
{
public:
    LegacyPassManagerOptimizer(unsigned opt_level)
    {
        m_builder.OptLevel = opt_level;
    }

    llvm::Expected<llvm::orc::ThreadSafeModule> operator()(
        llvm::orc::ThreadSafeModule thread_safe_module,
        llvm::orc::MaterializationResponsibility const& responsibility);

private:
    llvm::PassManagerBuilder m_builder;
};

}  // namespace engine
}  // namespace routing

#endif
