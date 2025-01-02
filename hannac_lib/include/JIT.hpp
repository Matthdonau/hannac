#ifndef JIT_HPP
#define JIT_HPP

// llvm includes.
#include "llvm/ExecutionEngine/JITSymbol.h"
#include "llvm/ExecutionEngine/Orc/CompileUtils.h"
#include "llvm/ExecutionEngine/Orc/Core.h"
#include "llvm/ExecutionEngine/Orc/ExecutionUtils.h"
#include "llvm/ExecutionEngine/Orc/ExecutorProcessControl.h"
#include "llvm/ExecutionEngine/Orc/JITTargetMachineBuilder.h"
#include "llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h"
#include "llvm/ExecutionEngine/Orc/Shared/ExecutorSymbolDef.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/LLVMContext.h"

// stdlib includes.
#include <memory> // unique_ptr

namespace hannac
{
namespace jit
{

class JITSingelton final
{
  public:
    static JITSingelton &get_jit()
    {
        static JITSingelton jit = JITSingelton::make_jit();
        return jit;
    }

    // Create JIT instance.
    static JITSingelton make_jit()
    {
        // System setup.
        llvm::InitializeNativeTarget();
        llvm::InitializeNativeTargetAsmPrinter();
        llvm::InitializeNativeTargetAsmParser();

        // Control over process memory.
        static llvm::ExitOnError err;
        // Create execution session.
        auto executionSession =
            std::make_unique<llvm::orc::ExecutionSession>(err(llvm::orc::SelfExecutorProcessControl::Create()));

        // Build target machine.
        llvm::orc::JITTargetMachineBuilder targetMachine(
            executionSession->getExecutorProcessControl().getTargetTriple());

        // Build data layout.
        auto dataLayout = std::make_unique<llvm::DataLayout>(err(targetMachine.getDefaultDataLayoutForTarget()));

        // Build object layer.
        auto objectLayer = std::make_unique<llvm::orc::RTDyldObjectLinkingLayer>(
            *executionSession, []() { return std::make_unique<llvm::SectionMemoryManager>(); });
        if (targetMachine.getTargetTriple().isOSBinFormatCOFF())
        {
            objectLayer->setOverrideObjectFlagsWithResponsibilityFlags(true);
            objectLayer->setAutoClaimResponsibilityForObjectSymbols(true);
        }

        // Build compilation layer.
        auto compilationLayer = std::make_unique<llvm::orc::IRCompileLayer>(
            *executionSession, *objectLayer,
            std::make_unique<llvm::orc::ConcurrentIRCompiler>(std::move(targetMachine)));

        // Create lib.
        auto &lib = executionSession->createBareJITDylib("<main>");
        lib.addGenerator(
            err(llvm::orc::DynamicLibrarySearchGenerator::GetForCurrentProcess(dataLayout->getGlobalPrefix())));

        return JITSingelton(std::move(executionSession), std::move(dataLayout), std::move(objectLayer),
                            std::move(compilationLayer), lib);
    }

    const llvm::DataLayout get_data_layout() const noexcept
    {
        return *mDataLayout.get();
    }

    llvm::orc::ResourceTrackerSP create_ressource_tracker()
    {
        return mLib.createResourceTracker();
    }

    llvm::Error add_module(llvm::orc::ThreadSafeModule threadSafeModule,
                           llvm::orc::ResourceTrackerSP ressourceTracker = nullptr)
    {
        ressourceTracker = ressourceTracker == nullptr ? mLib.getDefaultResourceTracker() : ressourceTracker;
        return mCompilationLayer->add(ressourceTracker, std::move(threadSafeModule));
    }

    llvm::orc::ExecutorSymbolDef find_symbol(std::string name)
    {
        llvm::orc::MangleAndInterner mangler(*mExecutionSession, *mDataLayout);
        static llvm::ExitOnError err;
        auto res = err(mExecutionSession->lookup({&mLib}, mangler(name)));

        return res;
    }

    JITSingelton(const JITSingelton &) = delete;
    JITSingelton &operator=(const JITSingelton &) = delete;

  private:
    JITSingelton(std::unique_ptr<llvm::orc::ExecutionSession> executionSession,
                 std::unique_ptr<llvm::DataLayout> dataLayout,
                 std::unique_ptr<llvm::orc::RTDyldObjectLinkingLayer> objectLayer,
                 std::unique_ptr<llvm::orc::IRCompileLayer> compLayer, llvm::orc::JITDylib &lib)
        : mExecutionSession(std::move(executionSession)), mDataLayout(std::move(dataLayout)),
          mObjectLayer(std::move(objectLayer)), mCompilationLayer(std::move(compLayer)), mLib(lib)
    {
    }

    ~JITSingelton()
    {
        if (auto err = mExecutionSession->endSession())
            mExecutionSession->reportError(std::move(err));
    }

    // Runnning jit program.
    std::unique_ptr<llvm::orc::ExecutionSession> mExecutionSession;

    // Target data layout.
    std::unique_ptr<llvm::DataLayout> mDataLayout;

    std::unique_ptr<llvm::orc::RTDyldObjectLinkingLayer> mObjectLayer;
    std::unique_ptr<llvm::orc::IRCompileLayer> mCompilationLayer;

    // JIT dynamic library.
    llvm::orc::JITDylib &mLib;
};
} // namespace jit
} // namespace hannac

#endif // JIT_HPP