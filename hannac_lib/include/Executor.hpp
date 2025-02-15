#ifndef EXECUTOR_HPP
#define EXECUTOR_HPP

// stdlib includes.
#include <cstdint>
#include <iostream>
#include <memory>

// hannac includes.
#include "AST.hpp"
#include "Codegen.hpp"
#include "GlobalSettings.hpp"

namespace hannac
{

enum class HResultType : std::uint8_t
{
    INT = 0,
    REAL = 1
};

union res {
    double r;
    std::int64_t i;
};

class HResult
{
  public:
    HResult(HResultType type, res result) : mType{type}, mResult{result}
    {
    }
    res get_result() const noexcept
    {
        return mResult;
    }

    HResultType get_type() const noexcept
    {
        return mType;
    }

  private:
    HResultType mType;
    res mResult;
};

struct HProgramState
{
    std::vector<HResult> mResults;
    size_t mStep = 0;
};

/******************************************************************************
 ********************************* HELPERS ************************************
 *****************************************************************************/

inline void print_result(HResult &res)
{
    switch (res.get_type())
    {
    case HResultType::INT:
        std::cout << "\t" << "Result: " << res.get_result().i << std::endl;
        break;
    case HResultType::REAL:
        std::cout << "\t" << "Result: " << res.get_result().r << std::endl;
        break;
    }
    return;
}

/******************************************************************************
 ********************************* EXECUTOR ***********************************
 *****************************************************************************/

// Executes a hanna program.
// A vector of ast::Expressions is provided to the HExecutor which resembles the program steps in the correct order.
// HExecutor will execute the steps in order of the vector.
class HExecutor final
{
  public:
    explicit HExecutor(std::vector<std::unique_ptr<ast::Expression>> program) : mProgram(std::move(program))
    {
    }

    std::vector<HResult> operator()()
    {
        for (auto &line : mProgram)
        {
            auto declaration =
                std::make_shared<hannac::ast::MethodDeclaration>("__hanna_execution", std::vector<std::string>());

            if (HSettings::get_settings().get_verbose() > 0)
                std::cout << "Executing: " << line->get_call() << std::endl;

            auto method = std::make_unique<hannac::ast::MethodDefinition>(std::move(declaration), std::move(line));

            // Immediately execute artifical generated method.
            hannac::HResult result = execute(std::move(method));
            mState.mResults.push_back(result);

            if (HSettings::get_settings().get_verbose() > 0)
            {
                print_result(result);
                std::cout << std::endl;
            }
        }

        return mState.mResults;
    }

    HResult execute(std::unique_ptr<ast::MethodDefinition> method)
    {
        // Generate code.
        auto code = method->codegen();
        if (HSettings::get_settings().get_verbose() > 1)
        {
            std::cout << "Executing " << method->get_name() << std::endl;
            code->print(llvm::outs());
        }

        // Create ressource tracker for execution method.
        auto ressourceTracker = jit::JITSingelton::get_jit().create_ressource_tracker();
        gen_module_and_reset(ressourceTracker);

        // Execute newly generated method by finding its symbol, getting its adress and calling it.
        auto ExprSymbol = jit::JITSingelton::get_jit().find_symbol("__hanna_execution");

        // Generate module, add function and reset module.
        static llvm::ExitOnError err;
        if (method->get_return_type() == ast::ASTType::RealNumber)
        {
            // works because double and int64_t are 64Bit.
            double (*call)() = ExprSymbol.getAddress().toPtr<double (*)()>();
            HResult dRes{HResultType::REAL, res{call()}};

            // Delete the anonymous expression module from the JIT.
            err(ressourceTracker->remove());

            return dRes;
        }
        else
        {
            std::int64_t (*call)() = ExprSymbol.getAddress().toPtr<std::int64_t (*)()>();
            HResult iRes{HResultType::INT, res{.i = call()}};

            // Delete the anonymous expression module from the JIT.
            err(ressourceTracker->remove());

            return iRes;
        }
    }

  private:
    HProgramState mState;
    std::vector<std::unique_ptr<ast::Expression>> mProgram;
};
} // namespace hannac
#endif // EXECUTOR_HPP