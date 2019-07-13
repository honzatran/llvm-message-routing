
#include <memory>

#include <routing/engine/SimpleOrcJit.h>

#include <routing/engine/ClangCC1Driver.h>

#include <llvm/Support/CommandLine.h>
#include <llvm/Support/PrettyStackTrace.h>
#include <llvm/Support/Error.h>
#include <llvm/Support/Signals.h>
#include <llvm/Support/TargetSelect.h>

#include <iostream>

// Show the error message and exit.
LLVM_ATTRIBUTE_NORETURN static void fatalError(llvm::Error E) {

  llvm::handleAllErrors(std::move(E), [&](const llvm::ErrorInfoBase &EI) {
    std::cerr << "Fatal Error: ";
    EI.log(llvm::errs());
    std::cerr << "\n";
    std::cerr.flush();
  });

  exit(1);
}

// Determine the size of a C array at compile-time.
template <typename T, size_t sizeOfArray>
constexpr unsigned arrayElements(T (&)[sizeOfArray]) {
  return sizeOfArray;
}

// This function will be called from JITed code.
// DECL_JIT_ACCESS_CPP int *customIntAllocator(unsigned items) {
//   static int memory[100];
//   static unsigned allocIdx = 0;
//
//   if (allocIdx + items > arrayElements(memory))
//     exit(-1);
//
//   int *block = memory + allocIdx;
//   allocIdx += items;
//
//   return block;
// }
//


int main(int argc, char **argv) {
  using namespace llvm;

  sys::PrintStackTraceOnErrorSignal(argv[0]);
  PrettyStackTraceProgram X(argc, argv);

  atexit(llvm_shutdown);

  InitializeNativeTarget();
  InitializeNativeTargetAsmPrinter();
  InitializeNativeTargetAsmParser();

  // Parse -debug and -debug-only options.
  cl::ParseCommandLineOptions(argc, argv, "JitFromScratch example project\n");

  int x[]{0, 1, 2};
  int y[]{3, 1, -1};

  // auto targetMachine = EngineBuilder().selectTarget();
  // auto jit = std::make_unique<SimpleOrcJit>(*targetMachine);

  // Implementation of the integerDistances function.
  std::string sourceCode =
      "extern \"C\" int f() { return 42; } \n";

  // Compile C++ to bitcode.
  LLVMContext context;
  ClangCC1Driver driver;
  auto module = driver.compileTranslationUnit(sourceCode, context);
  if (!module)
  {
      fatalError(module.takeError());
  }

  auto jitExpected = SimpleOrcJit::create();

  if (!jitExpected)
  {
      fatalError(jitExpected.takeError());
  }

  auto jit = std::move(jitExpected.get());

  auto result = jit->add(std::move(*module));

  if (result)
  {
      fatalError(std::move(result));
  }

  auto function_f = jit->lookup("f");

  if (!function_f) 
  {
      fatalError(function_f.takeError());
  }

  std::cout << "EVALUATING" << std::endl;

  int (*f) () = (int (*)()) (function_f->getAddress());

  std::cout << (*f)() << std::endl;

  return 0;
}
