#include <routing/engine/clang_cc1_driver.h>

// Hack: cc1 lives in "tools" next to "include"
#include <../tools/driver/cc1_main.cpp>

#if _WIN32
#include <routing/engine/ClangCC1Args_Win.h>
#elif __APPLE__
#include <routing/engine/ClangCC1Args_OSX.h>
#elif __linux__
#include <routing/engine/ClangCC1Args_Linux.h>
#endif

#include <llvm/ADT/SmallString.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Bitcode/BitcodeReader.h>
#include <llvm/Support/Error.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/MemoryBuffer.h>

#define DEBUG_TYPE "cc1driver"

namespace
{
llvm::Error
return_code_error(llvm::StringRef message, int returnCode)
{
    return llvm::make_error<llvm::StringError>(
        message, std::error_code(returnCode, std::system_category()));
}

llvm::Expected<std::string>
saveSourceFile(std::string content)
{
    using llvm::sys::fs::createTemporaryFile;

    int fd;
    llvm::SmallString<128> name;
    if (auto ec = createTemporaryFile("JitFromScratch", "cpp", fd, name))
        return llvm::errorCodeToError(ec);

    constexpr bool shouldClose = true;
    constexpr bool unbuffered  = true;
    llvm::raw_fd_ostream os(fd, shouldClose, unbuffered);
    os << content;

    return name.str();
}

std::string
replaceExtension(llvm::StringRef name, llvm::StringRef ext)
{
    return name.substr(0, name.find_last_of('.') + 1).str() + ext.str();
}

llvm::Error
compileCppToBitcodeFile(std::vector<std::string> args)
{
    // DEBUG({
    //   llvm::dbgs() << "Invoke Clang cc1 with args:\n";
    //   for (std::string arg : args)
    //     llvm::dbgs() << arg << " ";
    //   llvm::dbgs() << "\n\n";
    // });

    std::vector<const char *> argsX;
    std::transform(
        args.begin(),
        args.end(),
        std::back_inserter(argsX),
        [](const std::string &s) { return s.c_str(); });

    if (int res = cc1_main(argsX, "", nullptr))
        return return_code_error("Clang cc1 compilation failed", res);

    return llvm::Error::success();
}

llvm::Expected<std::unique_ptr<llvm::Module>>
readModuleFromBitcodeFile(llvm::StringRef bc, llvm::LLVMContext &context)
{
    llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> buffer
        = llvm::MemoryBuffer::getFile(bc);

    if (!buffer)
        return llvm::errorCodeToError(buffer.getError());

    return llvm::parseBitcodeFile(buffer.get()->getMemBufferRef(), context);
}

}  // end namespace

llvm::Expected<std::unique_ptr<llvm::Module>>
Clang_cc1_driver::compileTranslationUnit(
    std::string cppCode,
    llvm::LLVMContext &context)
{
    auto sourceFileName = saveSourceFile(cppCode);
    if (!sourceFileName)
        return sourceFileName.takeError();

    std::string cpp = *sourceFileName;
    std::string bc  = replaceExtension(cpp, "bc");

    llvm::Error err = compileCppToBitcodeFile(getClangCC1Args(cpp, bc));
    if (err)
        return std::move(err);

    auto module = readModuleFromBitcodeFile(bc, context);
    llvm::sys::fs::remove(bc);

    if (!module)
    {
        llvm::sys::fs::remove(cpp);
        return module.takeError();
    }

    SoucreFileDeleters.push_back([cpp]() { llvm::sys::fs::remove(cpp); });

    return std::move(*module);
}

llvm::Expected<std::unique_ptr<llvm::Module>>
Clang_cc1_driver::compile_source_code(
    std::string const &source_code_path,
    llvm::LLVMContext &context)
{
    std::string bc  = replaceExtension(source_code_path, "bc");

    llvm::Error err = compileCppToBitcodeFile(getClangCC1Args(source_code_path, bc));
    if (err)
        return std::move(err);

    auto module = readModuleFromBitcodeFile(bc, context);
    llvm::sys::fs::remove(bc);

    if (!module)
    {
        return module.takeError();
    }

    return std::move(*module);
}
