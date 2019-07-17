

#include <routing/engine/SimpleOrcJit.h>
#include <routing/file_util.h>
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/LoopAnalysisManager.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/ExecutionEngine/Orc/ThreadSafeModule.h"
#include "llvm/IR/Mangler.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/PassAnalysisSupport.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Instrumentation.h"

#include <llvm/IR/LegacyPassManager.h>

#include <routing/logger.h>

#include <iostream>
#include <memory>

int
g()
{
    return 42;
}

llvm::Expected<llvm::orc::ThreadSafeModule>
SimpleOrcJit::optimize_module(
    llvm::orc::ThreadSafeModule tsm,
    llvm::orc::MaterializationResponsibility const&
        materialization_responsibility)
{
    // llvm::PassBuilder builder;
    //
    // llvm::ModuleAnalysisManager analysis_manager(true);
    // llvm::FunctionAnalysisManager funtion_analysis_manager(true);
    // llvm::LoopAnalysisManager loop_analysis_manager(true);
    // llvm::CGSCCAnalysisManager cgsc_analysis_manager(true);
    //
    // builder.registerModuleAnalyses(analysis_manager);
    // builder.registerFunctionAnalyses(funtion_analysis_manager);
    // builder.registerLoopAnalyses(loop_analysis_manager);
    // builder.registerCGSCCAnalyses(cgsc_analysis_manager);
    //
    // builder.crossRegisterProxies(
    //     loop_analysis_manager,
    //     funtion_analysis_manager,
    //     cgsc_analysis_manager,
    //     analysis_manager);

    // funtion_analysis_manager.registerPass([] { return
    // llvm::PassInstrumentationAnalysis(); });
    // funtion_analysis_manager.registerPass([] { return
    // llvm::TargetIRAnalysis(); });
    // // funtion_analysis_manager.registerPass([&loop_analysis_manager] {
    // //         return
    // llvm::LoopAnalysisManagerFunctionProxy(loop_analysis_manager);
    // // });
    //
    // llvm::FunctionAnalysisManager m(true);
    //
    // loop_analysis_manager.registerPass([&m] {
    //         return llvm::FunctionAnalysisManagerLoopProxy(m);
    // });
    //
    // analysis_manager.registerPass([] { return llvm::TargetLibraryAnalysis();
    // }); analysis_manager.registerPass([] { return
    // llvm::PassInstrumentationAnalysis(); });
    // analysis_manager.registerPass([&funtion_analysis_manager] {
    //         return
    //         llvm::FunctionAnalysisManagerModuleProxy(funtion_analysis_manager);
    // });

    // auto modulePassManager
    //     = builder.buildModuleOptimizationPipeline(llvm::PassBuilder::O3);
    // modulePassManager.run(*tsm.getModule(), analysis_manager);

    llvm::Module& module = *tsm.getModule();

    llvm::legacy::FunctionPassManager function_manager(&module);

    std::cout << "OPTIMIZATION" << std::endl;

    return llvm::Expected<llvm::orc::ThreadSafeModule>(std::move(tsm));
}

llvm::Expected<std::unique_ptr<llvm::Module>>
SimpleOrcJit::compuleModuleFromFile(
    std::string const& file_path,
    llvm::LLVMContext& context)
{
    std::string source_code = routing::read_file(file_path.c_str());
    return compileModuleFromCpp(source_code, context);
}

llvm::Expected<llvm::JITEvaluatedSymbol>
SimpleOrcJit::lookup(llvm::StringRef name)
{
    std::string mangled_name;

    llvm::raw_string_ostream output(mangled_name);

    llvm::Mangler::getNameWithPrefix(output, name, m_data_layout);

    auto tmp = m_execution_session.intern(output.str());

    return m_execution_session.lookup(
        {&m_execution_session.getMainJITDylib()}, tmp);
}
