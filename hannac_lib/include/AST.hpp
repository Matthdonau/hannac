#ifndef AST_HPP
#define AST_HPP

// stdlib includes
#include <cstdint>
#include <iostream>
#include <memory>
#include <vector>

// hanna includes.
#include "Codegen.hpp"
#include "GlobalSettings.hpp"

// llvm includes
#include "llvm/IR/Verifier.h"

namespace hannac
{
namespace ast
{
// Types of AST nodes available in hanna.
enum class ASTType
{
    Number = 0,
    RealNumber = 1,
    Variable = 2,
    Binary = 3,
    FuncDecl = 4,
    FuncDef = 5,
    MethodCall = 6
};

static std::string ast_to_string(ASTType type)
{
    switch (type)
    {
    case ASTType::Number:
        return "int";
    case ASTType::RealNumber:
        return "double";
    case ASTType::Variable:
        return "var";
    default:
        return "";
    }
}

std::string produce_func_name(std::string &name, std::vector<ASTType> argTypes)
{
    std::string funcName{name};
    for (auto const &el : argTypes)
    {
        funcName += "_" + ast_to_string(el);
    }
    return funcName;
}

// Function AST buffer.
// Code for functions is generated lazily, i.e. only when they are actually called.
// From definition of the function to its first use, store the AST Node in this buffer.
// The argument and return type of the function is determined by the arguments it is called with.
// Much like template functions in C++. Therefore storing only the name as key suffices.
struct FunctionDefinition; // Forward declaration
class HMethodBuffer final
{
  public:
    static std::map<std::string, std::shared_ptr<FunctionDefinition>> &get()
    {
        static HMethodBuffer buffer;
        return buffer.funcMap;
    }

    HMethodBuffer(const HMethodBuffer &) = delete;
    HMethodBuffer &operator=(const HMethodBuffer &) = delete;

    std::map<std::string, std::shared_ptr<FunctionDefinition>> funcMap;

  private:
    HMethodBuffer() {};
};

// Since we are putting code for each function in a separate module, we need a way for subsequent calls to functions to
// gather the function declaration. This is done by storing the function declarations produced so far here.
struct FunctionDeclaration; // Forward declaration
class HFuncDeclarations final
{
  public:
    static std::map<std::string, std::pair<std::shared_ptr<hannac::ast::FunctionDeclaration>, ASTType>> &get()
    {
        static HFuncDeclarations funcMap;
        return funcMap.mFunctions;
    }

    HFuncDeclarations(const HFuncDeclarations &) = delete;
    HFuncDeclarations &operator=(const HFuncDeclarations &) = delete;
    std::map<std::string, std::pair<std::shared_ptr<hannac::ast::FunctionDeclaration>, ASTType>> mFunctions;

  private:
    HFuncDeclarations() = default;
};

/******************************************************************************
 *********************************** AST **************************************
 *****************************************************************************/
// Base class for all AST nodes.
struct Expression
{
  public:
    explicit Expression(ASTType type) : mType{type}
    {
    }
    Expression(Expression const &) = default;
    Expression(Expression &&) = default;
    Expression &operator=(Expression const &) = default;
    Expression &operator=(Expression &&) = default;
    virtual ~Expression() = default;

    virtual std::string get_name() const noexcept = 0;

    // Codegen.
    virtual llvm::Value *codegen() = 0;

    ASTType get_type() const noexcept
    {
        return mType;
    }

    virtual ASTType get_return_type() const noexcept
    {
        return ASTType::Number;
    }

  private:
    ASTType mType;
};

/******************************************************************************
 ******************************** Variables ***********************************
 *****************************************************************************/
// Literals
struct Number final : public Expression
{
  public:
    explicit Number(std::int64_t const &number) : Expression{ASTType::Number}, mNum{number}
    {
    }

    // Codegen.
    virtual llvm::Value *codegen() final
    {
        return llvm::ConstantInt::get(*HContextSingelton::get_context().mContext, llvm::APInt(64, mNum, true));
    }

    virtual std::string get_name() const noexcept override
    {
        return "Int Literal";
    }

  private:
    std::int64_t mNum;
};

struct RealNumber final : public Expression
{
  public:
    explicit RealNumber(double const &number) : Expression{ASTType::RealNumber}, mNum{number}
    {
    }

    // Codegen.
    virtual llvm::Value *codegen() final
    {
        return llvm::ConstantFP::get(*HContextSingelton::get_context().mContext, llvm::APFloat(mNum));
    }

    virtual std::string get_name() const noexcept override
    {
        return "Real Literal";
    }

  private:
    double mNum;
};

// Variable. Currently only int64_t possible.
struct Variable final : public Expression
{
  public:
    explicit Variable(std::string const &name) : Expression{ASTType::Variable}, mName{name}
    {
    }

    // Codegen.
    virtual llvm::Value *codegen() final
    {
        if (HNamesMap::get().find(mName) == HNamesMap::get().end())
            std::cout << "Unkown variable referenced." << std::endl;
        else
            return HNamesMap::get()[mName];

        return nullptr;
    }

    virtual std::string get_name() const noexcept override
    {
        return mName;
    }

  private:
    std::string mName;
};

/******************************************************************************
 ******************************** Opertations *********************************
 *****************************************************************************/
// Binary operation.
struct Binary final : public Expression
{
  public:
    Binary(char op, std::unique_ptr<Expression> left, std::unique_ptr<Expression> right)
        : Expression{ASTType::Binary}, mOperator{op}, mLHS(std::move(left)), mRHS(std::move(right)),
          mReturnType{ASTType::Number}
    {
    }

    // Codegen.
    virtual llvm::Value *codegen() final
    {
        // mLHS, mRHS get_name and assosicate with type.
        // ToDo
        auto left = mLHS->codegen();
        auto right = mRHS->codegen();

        if (left == nullptr || right == nullptr)
            return nullptr;

        // Determine from lhs and rhs which data type to generate code for.
        auto lType = left->getType()->getTypeID();
        auto rType = right->getType()->getTypeID();

        mReturnType = (lType == llvm::Type::DoubleTyID || rType == llvm::Type::DoubleTyID) ? ASTType::RealNumber
                                                                                           : ASTType::Number;

        switch (mOperator)
        {
        case '+':
            if (mReturnType == ASTType::RealNumber)
                return HBuilderSingelton::get_builder().mBuilder->CreateFAdd(left, right, "dadd");
            else
                return HBuilderSingelton::get_builder().mBuilder->CreateAdd(left, right, "add");
        case '-':
            if (mReturnType == ASTType::RealNumber)
                return HBuilderSingelton::get_builder().mBuilder->CreateFSub(left, right, "dsub");
            else
                return HBuilderSingelton::get_builder().mBuilder->CreateSub(left, right, "sub");
        case '*':
            if (mReturnType == ASTType::RealNumber)
                return HBuilderSingelton::get_builder().mBuilder->CreateFMul(left, right, "dmull");
            else
                return HBuilderSingelton::get_builder().mBuilder->CreateMul(left, right, "mull");
        case '/':
            if (mReturnType == ASTType::RealNumber)
                return HBuilderSingelton::get_builder().mBuilder->CreateFDiv(left, right, "ddiv");
            else
                return HBuilderSingelton::get_builder().mBuilder->CreateSDiv(left, right, "div");
        default:
            std::cout << "Unknown binary operator provided." << std::endl;
            return nullptr;
        }
    }

    virtual std::string get_name() const noexcept override
    {
        return {mOperator};
    }

    virtual ASTType get_return_type() const noexcept override
    {
        return mReturnType;
    }

  private:
    char mOperator;                   // binary operator.
    std::unique_ptr<Expression> mLHS; // Left argument.
    std::unique_ptr<Expression> mRHS; // Right argument.
    ASTType mReturnType;
};

/******************************************************************************
 ********************************* Functions **********************************
 *****************************************************************************/
// Function declaration.
struct FunctionDeclaration final : public Expression
{
  public:
    FunctionDeclaration(std::string const &name, std::vector<std::string> const &args)
        : Expression{ASTType::FuncDecl}, mName{name}, mArguments{args}, mReturnType{ASTType::Number}
    {
    }

    // Codegen.
    virtual llvm::Function *codegen() final
    {
        llvm::FunctionType *proto;
        std::vector<llvm::Type *> types;

        if (mArgTypes.size() != mArguments.size())
        {
            std::cout << "Mismatch between argument definition and arguments provided to function call" << std::endl;
            return nullptr;
        }

        // Create types for input arguments.
        size_t i = 0;
        for (auto const &el : mArgTypes)
        {
            if (el == ASTType::RealNumber)
                types.push_back(llvm::Type::getDoubleTy(*HContextSingelton::get_context().mContext));
            else
                types.push_back(llvm::Type::getInt64Ty(*HContextSingelton::get_context().mContext));
        }

        // Create function prototype.
        if (mReturnType == ASTType::RealNumber)
            proto = llvm::FunctionType::get(llvm::Type::getDoubleTy(*HContextSingelton::get_context().mContext), types,
                                            false);
        else
            proto = llvm::FunctionType::get(llvm::Type::getInt64Ty(*HContextSingelton::get_context().mContext), types,
                                            false);

        // Create func.
        llvm::Function *func =
            llvm::Function::Create(proto, llvm::Function::ExternalLinkage, produce_func_name(mName, mArgTypes),
                                   HModuleSingelton::get_module().mModule.get());

        // Set argument names.
        i = 0;
        for (auto &arg : func->args())
            arg.setName(mArguments[i++]);

        return func;
    }

    std::string get_name() const noexcept override
    {
        return mName;
    }

    void set_arg_types(std::vector<ASTType> argTypes) noexcept
    {
        mArgTypes = argTypes;
    }

    void set_return_type(ASTType rType) noexcept
    {
        mReturnType = rType;
    }

    std::vector<std::string> get_arguments()
    {
        return mArguments;
    }

    virtual ASTType get_return_type() const noexcept override
    {
        return mReturnType;
    }

  private:
    std::string mName;
    std::vector<std::string> mArguments;
    // Updated once we know.
    std::vector<ASTType> mArgTypes;
    ASTType mReturnType;
};

llvm::Function *gen_func_decl(std::string &name, std::vector<ASTType> argTypes, ASTType returnType)
{

    auto func = HModuleSingelton::get_module().mModule->getFunction(produce_func_name(name, argTypes));
    if (func != nullptr)
        return func;

    auto proto = HFuncDeclarations::get().find(produce_func_name(name, argTypes));
    if (proto != HFuncDeclarations::get().end())
    {
        std::get<0>(proto->second)->set_arg_types(argTypes);
        std::get<0>(proto->second)->set_return_type(returnType);
        return std::get<0>(proto->second)->codegen();
    }

    return nullptr;
}

// Function call.
struct MethodCall final : public Expression
{
  public:
    MethodCall(std::string const &name, std::vector<std::unique_ptr<Expression>> args)
        : Expression{ASTType::MethodCall}, mName{name}, mArguments(std::move(args)), mReturnType{ASTType::Number}
    {
    }

    // Codegen.
    virtual llvm::Value *codegen() final
    {
        // Lookup function name first.
        mReturnType = std::get<1>(HFuncDeclarations::get().find(produce_func_name(mName, get_argtypes()))->second);

        llvm::Function *func = gen_func_decl(mName, get_argtypes(), mReturnType);
        if (HSettings::get_settings().get_verbose())
            std::cout << produce_func_name(mName, get_argtypes()) << std::endl;
        if (func == nullptr)
        {
            std::cout << "Unknown reference to function: " << mName << std::endl;
            return nullptr;
        }

        if (func->arg_size() != mArguments.size())
        {
            std::cout << "Incorrect number of arguments for function: " << mName << ". Expected " << func->arg_size()
                      << " but got " << mArguments.size() << std::endl;
            return nullptr;
        }

        std::vector<llvm::Value *> args;
        for (auto const &el : mArguments)
        {
            args.push_back(el->codegen());
        }

        // Since this method call is attached to non type version of the calling function reset its type back.
        mArgTypes = {};
        return HBuilderSingelton::get_builder().mBuilder->CreateCall(func, args, "funccall");
    }

    std::string get_name() const noexcept override
    {
        return mName;
    }

    // In order to produce code for the correct data types, we need to be able to query the types of the input arguments
    // of a function call.
    std::vector<ASTType> get_argtypes()
    {
        if (!mArgTypes.empty())
            return mArgTypes;

        std::vector<ASTType> ret;
        for (auto const &el : mArguments)
            ret.push_back(el->get_type());

        mArgTypes = ret;
        return ret;
    }

    void set_arg_types(std::vector<ASTType> &args)
    {
        mArgTypes = args;
        return;
    }

    std::vector<std::string> getArgNames()
    {
        std::vector<std::string> names;
        for (auto &el : mArguments)
        {
            names.push_back(el->get_name());
        }

        return names;
    }

    virtual ASTType get_return_type() const noexcept override
    {
        return mReturnType;
    }

  private:
    std::string mName;
    std::vector<std::unique_ptr<Expression>> mArguments;
    std::vector<ASTType> mArgTypes;
    ASTType mReturnType;
};

// Function definition.
struct FunctionDefinition final : public Expression
{
  public:
    FunctionDefinition(std::shared_ptr<FunctionDeclaration> def, std::unique_ptr<Expression> expr)
        : Expression{ASTType::FuncDef}, mDeclaration(def), mFuncBody(std::move(expr)), mReturnType{ASTType::Number}
    {
    }

    std::string get_name() const noexcept override
    {
        return mDeclaration->get_name();
    }

    // Codegen.
    virtual llvm::Function *codegen() final
    {
        // For the scope of this method the arguments this function is called with need to exist.

        // In case expression to evaluate for this function is itself a function call,
        // generate this called function only now.
        llvm::Value *ret = nullptr;
        std::vector<std::shared_ptr<llvm::Argument>> vec;
        std::shared_ptr<llvm::Argument> llvmVal;
        if (mFuncBody->get_type() == ASTType::MethodCall)
        {
            gen_buffered_func();
        }
        else // Expression
        {
            HNamesMap::get().clear();
            size_t i = 0;
            for (auto const &el : mDeclaration->get_arguments())
            {

                vec.push_back(llvmVal);
                if (mArgTypes[i++] == ASTType::RealNumber)
                    llvmVal = std::make_shared<llvm::Argument>(
                        llvm::Type::getDoubleTy(*HContextSingelton::get_context().mContext), el);
                else
                    llvmVal = std::make_shared<llvm::Argument>(
                        llvm::Type::getInt64Ty(*HContextSingelton::get_context().mContext), el);

                HNamesMap::get()[std::string(el)] = llvmVal.get();
            }
            // Generate expression in order to know return type.
            ret = mFuncBody->codegen();
            // Set return type.
            mDeclaration->set_return_type(mFuncBody->get_return_type());
            mReturnType = mFuncBody->get_return_type();
        }

        // Produce function declaration.
        std::string name = get_name();
        HFuncDeclarations::get()[produce_func_name(name, mArgTypes)] = {mDeclaration, mReturnType};
        llvm::Function *func = gen_func_decl(name, mArgTypes, mReturnType);
        if (func == nullptr)
            return nullptr;

        // Actually create function now.
        llvm::BasicBlock *block = llvm::BasicBlock::Create(*HContextSingelton::get_context().mContext, "Entry", func);
        HBuilderSingelton::get_builder().mBuilder->SetInsertPoint(block);

        // Add function args to name map.
        HNamesMap::get().clear();
        for (auto &arg : func->args())
            HNamesMap::get()[std::string(arg.getName())] = &arg;

        ret = mFuncBody->codegen();

        if (ret)
        {
            // Finish off the function.
            HBuilderSingelton::get_builder().mBuilder->CreateRet(ret);

            // Validate the generated code, checking for consistency.
            llvm::verifyFunction(*func);

            FPM fpm;
            fpm.mFuncPassManager->run(*func, *fpm.mFuncAnalysisManager);

            mDeclaration->set_return_type(mFuncBody->get_return_type());
            mReturnType = mFuncBody->get_return_type();

            return func;
        }

        // Error reading body, remove function.
        func->eraseFromParent();

        return nullptr;
    }

    void set_arg_types(std::vector<ASTType> argTypes) noexcept
    {
        mArgTypes = argTypes;
    }

    std::shared_ptr<FunctionDeclaration> get_decl()
    {
        return mDeclaration;
    }

    virtual ASTType get_return_type() const noexcept override
    {
        return mReturnType;
    }

    void set_return_type(ASTType type) noexcept
    {
        mReturnType = type;
        return;
    }

  private:
    void gen_buffered_func()
    {
        // Find function definition in global function buffer.
        auto funcAst = HMethodBuffer::get().find(mFuncBody->get_name());
        // If function definition exists, generate code.
        if (funcAst != HMethodBuffer::get().end())
        {
            auto funcCall = dynamic_cast<MethodCall *>(mFuncBody.get());

            // Set arguments. Loop is needed for call of calls.
            auto args = funcCall->get_argtypes();
            std::map<std::string, ASTType> argMap{};
            size_t i = 0;
            for (auto const &el : mArgTypes)
            {
                argMap.insert({mDeclaration->get_arguments()[i++], el});
            }

            std::vector<ASTType> inter;
            for (auto const &el : funcCall->getArgNames())
            {
                auto type = argMap.find(el);
                if (el != "Int Literal" && el != "Real Literal" && type == argMap.end())
                {
                    if (el != "Int Literal" && el != "Real Literal")
                        std::cout << "Unkown varibale ->" << el << "<- used in call to " << funcCall->get_name()
                                  << std::endl;
                    inter.push_back(ASTType::Variable);
                }
                else
                {
                    inter.push_back(type->second);
                }
            }

            i = 0;
            for (auto &arg : args)
            {
                if (arg == ASTType::Variable)
                {
                    arg = inter[i];
                }
                i++;
            }

            funcAst->second->set_arg_types(args);
            funcCall->set_arg_types(args);
            // Only generate function if not already happened in previous call to it.
            auto name = funcCall->get_name();
            auto decl = HFuncDeclarations::get().find(produce_func_name(name, args));
            if (decl == HFuncDeclarations::get().end())
            {
                auto code = funcAst->second->codegen();
                mDeclaration->set_return_type(funcAst->second->get_return_type());
                mReturnType = funcAst->second->get_return_type();
                if (HSettings::get_settings().get_verbose())
                    code->print(llvm::outs());

                // Generate module for function, put code in and open new module.
                gen_module_and_reset();
            }
            else
            {
                mDeclaration->set_return_type(std::get<1>(decl->second));
                mReturnType = std::get<1>(decl->second);
            }
        }
        else
        {
            std::cout << "Referencing undefined function in call." << std::endl;
        }

        return;
    }

    std::shared_ptr<FunctionDeclaration> mDeclaration;
    std::unique_ptr<Expression> mFuncBody;
    std::vector<ASTType> mArgTypes;
    ASTType mReturnType;
};
} // namespace ast
} // namespace hannac
#endif // AST_HPP