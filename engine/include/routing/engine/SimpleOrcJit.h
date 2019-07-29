#include <clang/Driver/Options.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/ExecutionEngine/JITSymbol.h>
#include <llvm/ExecutionEngine/Orc/ThreadSafeModule.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/Error.h>

#include "clang_cc1_driver.h"
#include "llvm-c/TargetMachine.h"
#include "llvm/Support/FileSystem.h"
#include "routing/engine/optimizer.h"

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/JITEventListener.h>

#include <llvm/ExecutionEngine/Orc/CompileUtils.h>
#include <llvm/ExecutionEngine/Orc/Core.h>
#include <llvm/ExecutionEngine/Orc/ExecutionUtils.h>
#include <llvm/ExecutionEngine/Orc/IRCompileLayer.h>
#include <llvm/ExecutionEngine/Orc/IRTransformLayer.h>
#include <llvm/ExecutionEngine/Orc/LambdaResolver.h>
#include <llvm/ExecutionEngine/Orc/OrcError.h>
#include <llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h>
#include <llvm/ExecutionEngine/RuntimeDyld.h>
#include <llvm/ExecutionEngine/SectionMemoryManager.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Mangler.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Support/Debug.h>
#include <llvm/Support/DynamicLibrary.h>

#include <functional>
#include <iostream>
#include <memory>
#include <ostream>
#include <vector>

#define DEBUG_TYPE "jitfromscratch"

#if _WIN32
#define DECL_JIT_ACCESS_CPP __declspec(dllexport)
#else
#define DECL_JIT_ACCESS_CPP
#endif

#pragma once

class SimpleOrcJit
{
    // struct NotifyObjectLoaded_t {
    //   NotifyObjectLoaded_t(SimpleOrcJit &jit) : Jit(jit) {}
    //
    //   // Called by the ObjectLayer for each emitted object.
    //   // Forward notification to GDB JIT interface.
    //   void
    //   operator()(llvm::orc::RTDyldObjectLinkingLayer::ObjHandleT,
    //              const llvm::orc::RTDyldObjectLinkingLayerBase::ObjectPtr
    //              &obj, const llvm::LoadedObjectInfo &info) {
    //
    //     // Workaround 5.0 API inconsistency:
    //     // http://lists.llvm.org/pipermail/llvm-dev/2017-August/116806.html
    //     const auto &fixedInfo =
    //         static_cast<const llvm::RuntimeDyld::LoadedObjectInfo &>(info);
    //
    //     Jit.GdbEventListener->NotifyObjectEmitted(*obj->getBinary(),
    //     fixedInfo);
    //   }
    //
    // private:
    //   SimpleOrcJit &Jit;
    // };

public:
    SimpleOrcJit(
        llvm::orc::JITTargetMachineBuilder builder,
        llvm::DataLayout data_layout)
        : m_object_layer(
            m_execution_session,
            [] { return llvm::make_unique<llvm::SectionMemoryManager>(); }),
          m_compile_layer(
              m_execution_session,
              m_object_layer,
              llvm::orc::ConcurrentIRCompiler(std::move(builder))),
          m_data_layout(std::move(data_layout)),
          m_mangler(m_execution_session, this->m_data_layout),
          m_thread_safe_context(llvm::make_unique<llvm::LLVMContext>()),
          m_optimize_layer(
              m_execution_session,
              m_compile_layer,
              optimize_module)
    {
        // Load own executable as dynamic library.
        // Required for RTDyldMemoryManager::getSymbolAddressInProcess().
        // llvm::sys::DynamicLibrary::LoadLibraryPermanently(nullptr);

        // Internally points to a llvm::ManagedStatic.
        // No need to free. "create" is a misleading term here.
        // GdbEventListener =
        // llvm::JITEventListener::createGDBRegistrationListener();
        //
        auto system_process_generator
            = llvm::orc::DynamicLibrarySearchGenerator::GetForCurrentProcess(
                m_data_layout);

        if (!system_process_generator)
        {
            m_execution_session.reportError(
                system_process_generator.takeError());
            return;
        }

        std::cout << "SETTING GENERATOR" << std::endl;

        auto err = m_overrides.enable(m_execution_session.getMainJITDylib(), m_mangler);

        if (err)
        {
            std::move(err);
        }

        m_execution_session.getMainJITDylib().setGenerator(
            std::move(*system_process_generator));
    }

    static llvm::Expected<std::unique_ptr<SimpleOrcJit>> create()
    {
        auto jtmb = llvm::orc::JITTargetMachineBuilder::detectHost();

        if (!jtmb)
        {
            return jtmb.takeError();
        }

        auto data_layout = jtmb->getDefaultDataLayoutForTarget();

        if (!data_layout)
        {
            return data_layout.takeError();
        }

        return llvm::make_unique<SimpleOrcJit>(
            std::move(*jtmb), std::move(*data_layout));
    }

    llvm::Error add(std::unique_ptr<llvm::Module> module)
    {
        // Commit module for compilation to machine code. Actual compilation
        // happens on demand as soon as one of it's symbols is accessed. None of
        // the layers used here issue Errors from this call.
        //
        m_optimize_layer.setTransform(
            routing::engine::LegacyPassManagerOptimizer(3));

        return m_optimize_layer.add(
            m_execution_session.getMainJITDylib(),
            llvm::orc::ThreadSafeModule(
                std::move(module), m_thread_safe_context));
    }

    llvm::Expected<std::unique_ptr<llvm::Module>> compileModuleFromCpp(
        std::string cppCode,
        llvm::LLVMContext& context)
    {
        return m_clang_driver.compileTranslationUnit(cppCode, context);
    }

    /// Creates a module from a single cpp file
    /// \param file_path path to the cpp source code
    /// \param context llvm context
    llvm::Expected<std::unique_ptr<llvm::Module>> compuleModuleFromFile(
        std::string const& file_path,
        llvm::LLVMContext& context);

    llvm::Expected<llvm::JITEvaluatedSymbol> lookup(llvm::StringRef name);

    // template <class Signature_t>
    // llvm::Expected<std::function<Signature_t>> getFunction(std::string name)
    // {
    //   using namespace llvm;
    //
    //   // Find symbol name in committed modules.
    //   std::string mangledName = mangle(std::move(name));
    //   JITSymbol sym = findSymbolInJITedCode(mangledName);
    //   if (!sym)
    //     return make_error<orc::JITSymbolNotFound>(mangledName);
    //
    //   // Access symbol address.
    //   // Invokes compilation for the respective module if not compiled yet.
    //   Expected<JITTargetAddress> addr = sym.getAddress();
    //   if (!addr)
    //     return addr.takeError();
    //
    //   auto typedFunctionPtr = reinterpret_cast<Signature_t *>(*addr);
    //   return std::function<Signature_t>(typedFunctionPtr);
    // }

private:
    llvm::orc::ExecutionSession m_execution_session;
    llvm::orc::RTDyldObjectLinkingLayer m_object_layer;
    llvm::orc::IRCompileLayer m_compile_layer;
    llvm::DataLayout m_data_layout;
    llvm::orc::MangleAndInterner m_mangler;
    llvm::orc::ThreadSafeContext m_thread_safe_context;

    llvm::orc::IRTransformLayer m_optimize_layer;

    /// overrides for the C++ runtime
    llvm::orc::LocalCXXRuntimeOverrides m_overrides;

    Clang_cc1_driver m_clang_driver;

    static llvm::Expected<llvm::orc::ThreadSafeModule> optimize_module(
        llvm::orc::ThreadSafeModule tsm,
        llvm::orc::MaterializationResponsibility const&
            materialization_responsibility);

    // llvm::JITSymbol findSymbolInJITedCode(std::string mangledName) {
    //   constexpr bool exportedSymbolsOnly = false;
    //   return CompileLayer.findSymbol(mangledName, exportedSymbolsOnly);
    // }
    //
    // llvm::JITSymbol findSymbolInHostProcess(std::string mangledName) {
    //   // Lookup function address in the host symbol table.
    //   if (llvm::JITTargetAddress addr =
    //           llvm::RTDyldMemoryManager::getSymbolAddressInProcess(mangledName))
    //     return llvm::JITSymbol(addr, llvm::JITSymbolFlags::Exported);
    //
    //   return nullptr;
    // }
    //
    // // System name mangler: may prepend '_' on OSX or '\x1' on Windows
    // std::string mangle(std::string name) {
    //   std::string buffer;
    //   llvm::raw_string_ostream ostream(buffer);
    //   llvm::Mangler::getNameWithPrefix(ostream, std::move(name), DL);
    //   return ostream.str();
    // }
};
