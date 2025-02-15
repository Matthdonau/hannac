// stdlib includes
#include <cstdint>
#include <iostream>
#include <memory>
#include <vector>

// hanna includes.
#include "AST.hpp"
#include "Codegen.hpp"

namespace hannac
{
namespace ast
{
/******************************************************************************
 ******************************** Variables ***********************************
 *****************************************************************************/

/********************************* Int Number ********************************/
Number::Number(std::int64_t const &number) : Expression{ASTType::Number}, mNum{number}
{
}

// Codegen.
llvm::Value *Number::codegen()
{
    return llvm::ConstantInt::get(*HContextSingelton::get_context().mContext, llvm::APInt(64, mNum, true));
}

std::string Number::get_name() const noexcept
{
    return "Int Literal";
}

/****************************** Real Number ******************************/
RealNumber::RealNumber(double const &number) : Expression{ASTType::RealNumber}, mNum{number}
{
}

// Codegen.
llvm::Value *RealNumber::codegen()
{
    return llvm::ConstantFP::get(*HContextSingelton::get_context().mContext, llvm::APFloat(mNum));
}

std::string RealNumber::get_name() const noexcept
{
    return "Real Literal";
}

/******************************* Variable ********************************/
Variable::Variable(std::string const &name) : Expression{ASTType::Variable}, mName{name}
{
}

llvm::Value *Variable::codegen()
{
    if (HNamesMap::get().find(mName) == HNamesMap::get().end())
        std::cout << "Unknown variable referenced." << std::endl;
    else
        return HNamesMap::get()[mName];

    return nullptr;
}

std::string Variable::get_name() const noexcept
{
    return mName;
}

/******************************************************************************
 ******************************** Opertations *********************************
 *****************************************************************************/
// Binary operation.
Binary::Binary(char op, std::unique_ptr<Expression> left, std::unique_ptr<Expression> right)
    : Expression{ASTType::Binary}, mOperator{op}, mLHS(std::move(left)), mRHS(std::move(right)),
      mReturnType{ASTType::Number}
{
}

// Codegen.
llvm::Value *Binary::codegen()
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

    mReturnType =
        (lType == llvm::Type::DoubleTyID || rType == llvm::Type::DoubleTyID) ? ASTType::RealNumber : ASTType::Number;

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

std::string Binary::get_name() const noexcept
{
    return "Expression";
}

ASTType Binary::get_return_type() const noexcept
{
    return mReturnType;
}

/******************************************************************************
 ********************************** Methods ***********************************
 *****************************************************************************/

/******************************* Method declaration **************************/
MethodDeclaration::MethodDeclaration(std::string const &name, std::vector<std::string> const &args)
    : Expression{ASTType::FuncDecl}, mName{name}, mArguments{args}, mReturnType{ASTType::Number}
{
}

llvm::Function *MethodDeclaration::codegen()
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
        proto =
            llvm::FunctionType::get(llvm::Type::getDoubleTy(*HContextSingelton::get_context().mContext), types, false);
    else
        proto =
            llvm::FunctionType::get(llvm::Type::getInt64Ty(*HContextSingelton::get_context().mContext), types, false);

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

std::string MethodDeclaration::get_name() const noexcept
{
    return mName;
}

void MethodDeclaration::set_arg_types(std::vector<ASTType> argTypes) noexcept
{
    mArgTypes = argTypes;
}

void MethodDeclaration::set_return_type(ASTType rType) noexcept
{
    mReturnType = rType;
}

std::vector<std::string> MethodDeclaration::get_arguments()
{
    return mArguments;
}

ASTType MethodDeclaration::get_return_type() const noexcept
{
    return mReturnType;
}

/****************************** Method declaration *************************/
MethodDefinition::MethodDefinition(std::shared_ptr<MethodDeclaration> def, std::unique_ptr<Expression> expr)
    : Expression{ASTType::FuncDef}, mDeclaration(def), mFuncBody(std::move(expr)), mReturnType{ASTType::Number}
{
}

std::string MethodDefinition::get_name() const noexcept
{
    return mDeclaration->get_name();
}

// Codegen.
llvm::Function *MethodDefinition::codegen()
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
    HMethodDeclarations::get()[produce_func_name(name, mArgTypes)] = {mDeclaration, mReturnType};
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

void MethodDefinition::set_arg_types(std::vector<ASTType> argTypes) noexcept
{
    mArgTypes = argTypes;
}

std::shared_ptr<MethodDeclaration> MethodDefinition::get_decl()
{
    return mDeclaration;
}

ASTType MethodDefinition::get_return_type() const noexcept
{
    return mReturnType;
}

void MethodDefinition::set_return_type(ASTType type) noexcept
{
    mReturnType = type;
    return;
}

void MethodDefinition::gen_buffered_func()
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
        i = 0;
        for (auto &el : funcCall->getArgNames())
        {
            if (funcCall->get_argtypes()[i] == ASTType::MethodCall)
            {
                auto funcAst2 = HMethodBuffer::get().find(el);
                if (funcAst2 != HMethodBuffer::get().end())
                {
                    funcAst2->second->set_arg_types({});
                    std::vector<ASTType> tmp{};
                    auto decl = HMethodDeclarations::get().find(produce_func_name(el, tmp));
                    if (decl == HMethodDeclarations::get().end())
                    {
                        auto code = funcAst2->second->codegen();
                        // mDeclaration->set_return_type(funcAst->second->get_return_type());
                        // mReturnType = funcAst->second->get_return_type();
                        if (HSettings::get_settings().get_verbose())
                            code->print(llvm::outs());

                        // Generate module for function, put code in and open new module.
                        gen_module_and_reset();
                    }
                }
                inter.push_back(ASTType::MethodCall);
                i++;
                continue;
            }

            auto type = argMap.find(el);
            if (el != "Int Literal" && el != "Real Literal" && type == argMap.end())
            {
                if (el != "Int Literal" && el != "Real Literal" && el != "Expression")
                {
                    std::cout << "Unknown varibale ->" << el << "<- used in call to " << funcCall->get_name()
                              << std::endl;
                }
                if (el == "Expression")
                    inter.push_back(funcCall->get_argtypes()[i]);

                else
                    inter.push_back(ASTType::Variable);
            }
            else
            {
                inter.push_back(type->second);
            }
            i++;
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
        auto decl = HMethodDeclarations::get().find(produce_func_name(name, args));
        if (decl == HMethodDeclarations::get().end())
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

/******************************* Method call *****************************/
MethodCall::MethodCall(std::string const &name, std::vector<std::unique_ptr<Expression>> args)
    : Expression{ASTType::MethodCall}, mName{name}, mArguments(std::move(args)), mReturnType{ASTType::Number}
{
}

// Codegen.
llvm::Value *MethodCall::codegen()
{
    // Lookup function name first.
    mReturnType = std::get<1>(HMethodDeclarations::get().find(produce_func_name(mName, get_argtypes()))->second);

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

std::string MethodCall::get_name() const noexcept
{
    return mName;
}

// In order to produce code for the correct data types, we need to be able to query the types of the input arguments
// of a function call.
std::vector<ASTType> MethodCall::get_argtypes()
{
    if (!mArgTypes.empty())
        return mArgTypes;

    std::vector<ASTType> ret;
    for (auto const &el : mArguments)
    {
        if (el->get_type() == ASTType::Binary)
        {
            // What happens if expression is of variables.
            el->codegen();
            ret.push_back(el->get_return_type());
        }
        else
        {
            ret.push_back(el->get_type());
        }
    }

    mArgTypes = ret;
    return ret;
}

void MethodCall::set_arg_types(std::vector<ASTType> &args)
{
    mArgTypes = args;
    return;
}

std::vector<std::string> MethodCall::getArgNames()
{
    std::vector<std::string> names;
    for (auto &el : mArguments)
    {
        names.push_back(el->get_name());
    }

    return names;
}

ASTType MethodCall::get_return_type() const noexcept
{
    return mReturnType;
}
} // namespace ast
} // namespace hannac