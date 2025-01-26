#ifndef CODEGEN_HPP
#define CODEGEN_HPP

// stdlib includes
#include <iostream>
#include <map>
#include <memory>

// llvm includes.
#include "llvm/Analysis/CGSCCPassManager.h"
#include "llvm/Analysis/LoopAnalysisManager.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/StandardInstrumentations.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Transforms/Scalar/Reassociate.h"
#include "llvm/Transforms/Scalar/SimplifyCFG.h"

// hanna includes.
#include "JIT.hpp"

namespace hannac
{
class HContextSingelton final
{
  public:
    static HContextSingelton &get_context()
    {
        static HContextSingelton context;
        return context;
    }

    void reset()
    {
        mContext = std::make_unique<llvm::LLVMContext>();
        return;
    }

    HContextSingelton(const HContextSingelton &) = delete;
    HContextSingelton &operator=(const HContextSingelton &) = delete;

    std::unique_ptr<llvm::LLVMContext> mContext = std::make_unique<llvm::LLVMContext>();

  private:
    HContextSingelton() {};
};

class HBuilderSingelton final
{
  public:
    static HBuilderSingelton &get_builder()
    {
        static HBuilderSingelton builder;
        return builder;
    }

    void reset()
    {
        mBuilder = std::make_unique<llvm::IRBuilder<>>(*HContextSingelton::get_context().mContext);

        return;
    }

    static std::unique_ptr<llvm::IRBuilder<>> get()
    {
        return std::make_unique<llvm::IRBuilder<>>(*HContextSingelton::get_context().mContext);
    }

    HBuilderSingelton(const HBuilderSingelton &) = delete;
    HBuilderSingelton &operator=(const HBuilderSingelton &) = delete;

    std::unique_ptr<llvm::IRBuilder<>> mBuilder =
        std::make_unique<llvm::IRBuilder<>>(*HContextSingelton::get_context().mContext);

  private:
    HBuilderSingelton() {};
};

class HModuleSingelton final
{
  public:
    static HModuleSingelton &get_module()
    {
        static HModuleSingelton builder;
        return builder;
    }

    void reset()
    {
        mModule = std::make_unique<llvm::Module>("Hanna Jit", *HContextSingelton::get_context().mContext);
        mModule->setDataLayout(jit::JITSingelton::get_jit().get_data_layout());
        return;
    }

    HModuleSingelton(const HModuleSingelton &) = delete;
    HModuleSingelton &operator=(const HModuleSingelton &) = delete;

    std::unique_ptr<llvm::Module> mModule =
        std::make_unique<llvm::Module>("Hanna Jit", *HContextSingelton::get_context().mContext);

  private:
    HModuleSingelton()
    {

        mModule->setDataLayout(jit::JITSingelton::get_jit().get_data_layout());
    };
};

class FPM final
{
  public:
    FPM(const FPM &) = delete;
    FPM &operator=(const FPM &) = delete;

    std::unique_ptr<llvm::FunctionPassManager> mFuncPassManager = std::make_unique<llvm::FunctionPassManager>();
    std::unique_ptr<llvm::FunctionAnalysisManager> mFuncAnalysisManager =
        std::make_unique<llvm::FunctionAnalysisManager>();
    std::unique_ptr<llvm::LoopAnalysisManager> mLoopAnalysisManager = std::make_unique<llvm::LoopAnalysisManager>();
    std::unique_ptr<llvm::CGSCCAnalysisManager> mCGSCCAnalysisManager = std::make_unique<llvm::CGSCCAnalysisManager>();
    std::unique_ptr<llvm::ModuleAnalysisManager> mModAnalysisManager = std::make_unique<llvm::ModuleAnalysisManager>();
    std::unique_ptr<llvm::PassInstrumentationCallbacks> mPassInstCallbacl =
        std::make_unique<llvm::PassInstrumentationCallbacks>();
    std::unique_ptr<llvm::StandardInstrumentations> mStandardInst =
        std::make_unique<llvm::StandardInstrumentations>(*HContextSingelton::get_context().mContext,
                                                         /*DebugLogging*/ true);
    llvm::PassBuilder mPassBuilder;

    FPM()
    {
        mFuncPassManager->addPass(llvm::InstCombinePass());
        mFuncPassManager->addPass(llvm::ReassociatePass());
        mFuncPassManager->addPass(llvm::GVNPass());
        mFuncPassManager->addPass(llvm::SimplifyCFGPass());

        mStandardInst->registerCallbacks(*mPassInstCallbacl, mModAnalysisManager.get());
        mPassBuilder.registerModuleAnalyses(*mModAnalysisManager);
        mPassBuilder.registerFunctionAnalyses(*mFuncAnalysisManager);
        mPassBuilder.crossRegisterProxies(*mLoopAnalysisManager, *mFuncAnalysisManager, *mCGSCCAnalysisManager,
                                          *mModAnalysisManager);
    }
};

class HNamesMap final
{
  public:
    static std::map<std::string, llvm::Value *> &get()
    {
        static HNamesMap namesMap;
        return namesMap.mMap;
    }

    HNamesMap(const HNamesMap &) = delete;
    HNamesMap &operator=(const HNamesMap &) = delete;

    std::map<std::string, llvm::Value *> mMap;

  private:
    HNamesMap() = default;
};

inline void gen_module_and_reset(llvm::orc::ResourceTrackerSP rt = nullptr)
{
    // Put current state in JIT module, close it and open a new one for next function.
    static llvm::ExitOnError err;
    if (rt == nullptr)
        err(jit::JITSingelton::get_jit().add_module(llvm::orc::ThreadSafeModule(
            std::move(HModuleSingelton::get_module().mModule), std::move(HContextSingelton::get_context().mContext))));
    else
        err(jit::JITSingelton::get_jit().add_module(
            llvm::orc::ThreadSafeModule(std::move(HModuleSingelton::get_module().mModule),
                                        std::move(HContextSingelton::get_context().mContext)),
            rt));

    HContextSingelton::get_context().reset();
    HModuleSingelton::get_module().reset();
    HBuilderSingelton::get_builder().reset();

    return;
}
} // namespace hannac
#endif