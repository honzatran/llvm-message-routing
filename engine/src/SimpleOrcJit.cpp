

#include <routing/engine/SimpleOrcJit.h>
#include "llvm/Analysis/LoopAnalysisManager.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/PassAnalysisSupport.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Transforms/Instrumentation.h"

#include <iostream>

int g() {
    return 42;
}

llvm::Expected<llvm::orc::ThreadSafeModule> SimpleOrcJit::optimize_module(
      llvm::orc::ThreadSafeModule tsm, 
      llvm::orc::MaterializationResponsibility const& materialization_responsibility)
{
    llvm::PassBuilder builder;


    llvm::ModuleAnalysisManager analysis_manager(true);
    llvm::FunctionAnalysisManager funtion_analysis_manager(true);
    llvm::LoopAnalysisManager loop_analysis_manager(true);
    llvm::CGSCCAnalysisManager cgsc_analysis_manager(true);

    builder.registerModuleAnalyses(analysis_manager);
    builder.registerFunctionAnalyses(funtion_analysis_manager);
    builder.registerLoopAnalyses(loop_analysis_manager);
    builder.registerCGSCCAnalyses(cgsc_analysis_manager);

    builder.crossRegisterProxies(loop_analysis_manager, funtion_analysis_manager, cgsc_analysis_manager, analysis_manager);

    // funtion_analysis_manager.registerPass([] { return llvm::PassInstrumentationAnalysis(); });
    // funtion_analysis_manager.registerPass([] { return llvm::TargetIRAnalysis(); });
    // // funtion_analysis_manager.registerPass([&loop_analysis_manager] { 
    // //         return llvm::LoopAnalysisManagerFunctionProxy(loop_analysis_manager); 
    // // });
    //
    // llvm::FunctionAnalysisManager m(true);
    //
    // loop_analysis_manager.registerPass([&m] { 
    //         return llvm::FunctionAnalysisManagerLoopProxy(m); 
    // });
    //
    // analysis_manager.registerPass([] { return llvm::TargetLibraryAnalysis(); });
    // analysis_manager.registerPass([] { return llvm::PassInstrumentationAnalysis(); });
    // analysis_manager.registerPass([&funtion_analysis_manager] { 
    //         return llvm::FunctionAnalysisManagerModuleProxy(funtion_analysis_manager); 
    // });

    auto modulePassManager = builder.buildModuleOptimizationPipeline(llvm::PassBuilder::O3);
    modulePassManager.run(*tsm.getModule(), analysis_manager);

    std::cout << "OPTIMIZATION" << std::endl;

    return tsm;
}
