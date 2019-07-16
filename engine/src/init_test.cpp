

#include <routing/fmt.h>

#include <llvm/Support/Error.h>
#include <llvm/Support/PrettyStackTrace.h>
#include <llvm/Support/Signals.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/ManagedStatic.h>


/// Runs before the orc unit test is initialized 
void init_before_test(int argc, char** argv)
{
    using namespace llvm;

    sys::PrintStackTraceOnErrorSignal(argv[0]);
    PrettyStackTraceProgram X(argc, argv);

    atexit(llvm_shutdown);

    InitializeNativeTarget();
    InitializeNativeTargetAsmPrinter();
    InitializeNativeTargetAsmParser();
}

