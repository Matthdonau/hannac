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
    case ASTType::MethodCall:
        return "MethodCall";
    default:
        return "";
    }
}

inline std::string produce_func_name(std::string &name, std::vector<ASTType> argTypes)
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
struct MethodDefinition; // Forward declaration
class HMethodBuffer final
{
  public:
    static std::map<std::string, std::shared_ptr<MethodDefinition>> &get()
    {
        static HMethodBuffer buffer;
        return buffer.funcMap;
    }

    HMethodBuffer(const HMethodBuffer &) = delete;
    HMethodBuffer &operator=(const HMethodBuffer &) = delete;

    std::map<std::string, std::shared_ptr<MethodDefinition>> funcMap;

  private:
    HMethodBuffer() {};
};

// Since we are putting code for each function in a separate module, we need a way for subsequent calls to functions to
// gather the function declaration. This is done by storing the function declarations produced so far here.
struct MethodDeclaration; // Forward declaration
class HMethodDeclarations final
{
  public:
    static std::map<std::string, std::pair<std::shared_ptr<hannac::ast::MethodDeclaration>, ASTType>> &get()
    {
        static HMethodDeclarations funcMap;
        return funcMap.mFunctions;
    }

    HMethodDeclarations(const HMethodDeclarations &) = delete;
    HMethodDeclarations &operator=(const HMethodDeclarations &) = delete;
    std::map<std::string, std::pair<std::shared_ptr<hannac::ast::MethodDeclaration>, ASTType>> mFunctions;

  private:
    HMethodDeclarations() = default;
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
    explicit Number(std::int64_t const &number);
    Number(Number const &) = delete;
    Number(Number &&) = delete;
    Number &operator=(Number const &) = delete;
    Number &operator=(Number &&) = delete;
    ~Number() = default;

    // Codegen.
    virtual llvm::Value *codegen() final;

    virtual std::string get_name() const noexcept override;

  private:
    std::int64_t mNum;
};

struct RealNumber final : public Expression
{
  public:
    explicit RealNumber(double const &number);
    RealNumber(RealNumber const &) = delete;
    RealNumber(RealNumber &&) = delete;
    RealNumber &operator=(RealNumber const &) = delete;
    RealNumber &operator=(RealNumber &&) = delete;
    ~RealNumber() = default;

    // Codegen.
    virtual llvm::Value *codegen() final;

    virtual std::string get_name() const noexcept override;

  private:
    double mNum;
};

// Variable. Currently only int64_t and double possible.
struct Variable final : public Expression
{
  public:
    explicit Variable(std::string const &name);
    Variable(Variable const &) = delete;
    Variable(RealNumber &&) = delete;
    Variable &operator=(Variable const &) = delete;
    Variable &operator=(Variable &&) = delete;
    ~Variable() = default;

    // Codegen.
    virtual llvm::Value *codegen() final;

    virtual std::string get_name() const noexcept override;

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
    Binary(char op, std::unique_ptr<Expression> left, std::unique_ptr<Expression> right);
    Binary(Binary const &) = delete;
    Binary(Binary &&) = delete;
    Binary &operator=(Binary const &) = delete;
    Binary &operator=(Binary &&) = delete;
    ~Binary() = default;

    // Codegen.
    virtual llvm::Value *codegen() final;

    virtual std::string get_name() const noexcept override;

    virtual ASTType get_return_type() const noexcept override;

  private:
    char mOperator;                   // binary operator.
    std::unique_ptr<Expression> mLHS; // Left argument.
    std::unique_ptr<Expression> mRHS; // Right argument.
    ASTType mReturnType;
};

/******************************************************************************
 ********************************* Methods ************************************
 *****************************************************************************/
// Method declaration.
struct MethodDeclaration final : public Expression
{
  public:
    MethodDeclaration(std::string const &name, std::vector<std::string> const &args);
    MethodDeclaration(MethodDeclaration const &) = delete;
    MethodDeclaration(MethodDeclaration &&) = delete;
    MethodDeclaration &operator=(MethodDeclaration const &) = delete;
    MethodDeclaration &operator=(MethodDeclaration &&) = delete;
    ~MethodDeclaration() = default;

    // Codegen.
    virtual llvm::Function *codegen() final;

    std::string get_name() const noexcept override;

    void set_arg_types(std::vector<ASTType> argTypes) noexcept;

    void set_return_type(ASTType rType) noexcept;

    std::vector<std::string> get_arguments();

    virtual ASTType get_return_type() const noexcept override;

  private:
    std::string mName;
    std::vector<std::string> mArguments;
    // Updated once we know.
    std::vector<ASTType> mArgTypes;
    ASTType mReturnType;
};

// Method call.
struct MethodCall final : public Expression
{
  public:
    MethodCall(std::string const &name, std::vector<std::unique_ptr<Expression>> args);
    MethodCall(MethodCall const &) = delete;
    MethodCall(MethodCall &&) = delete;
    MethodCall &operator=(MethodCall const &) = delete;
    MethodCall &operator=(MethodCall &&) = delete;
    ~MethodCall() = default;

    // Codegen.
    virtual llvm::Value *codegen() final;

    std::string get_name() const noexcept override;
    // In order to produce code for the correct data types, we need to be able to query the types of the input arguments
    // of a function call.
    std::vector<ASTType> get_argtypes();

    void set_arg_types(std::vector<ASTType> &args);

    std::vector<std::string> getArgNames();

    virtual ASTType get_return_type() const noexcept override;

  private:
    std::string mName;
    std::vector<std::unique_ptr<Expression>> mArguments;
    std::vector<ASTType> mArgTypes;
    ASTType mReturnType;
};

// Method definition.
struct MethodDefinition final : public Expression
{
  public:
    MethodDefinition(std::shared_ptr<MethodDeclaration> def, std::unique_ptr<Expression> expr);
    MethodDefinition(MethodDefinition const &) = delete;
    MethodDefinition(MethodDefinition &&) = delete;
    MethodDefinition &operator=(MethodDefinition const &) = delete;
    MethodDefinition &operator=(MethodDefinition &&) = delete;
    ~MethodDefinition() = default;

    std::string get_name() const noexcept override;

    // Codegen.
    virtual llvm::Function *codegen() final;

    void set_arg_types(std::vector<ASTType> argTypes) noexcept;

    std::shared_ptr<MethodDeclaration> get_decl();

    virtual ASTType get_return_type() const noexcept override;

    void set_return_type(ASTType type) noexcept;

  private:
    void gen_buffered_func();

    std::shared_ptr<MethodDeclaration> mDeclaration;
    std::unique_ptr<Expression> mFuncBody;
    std::vector<ASTType> mArgTypes;
    ASTType mReturnType;
};

inline llvm::Function *gen_func_decl(std::string &name, std::vector<ASTType> argTypes, ASTType returnType)
{

    auto func = HModuleSingelton::get_module().mModule->getFunction(produce_func_name(name, argTypes));
    if (func != nullptr)
        return func;

    auto proto = HMethodDeclarations::get().find(produce_func_name(name, argTypes));
    if (proto != HMethodDeclarations::get().end())
    {
        std::get<0>(proto->second)->set_arg_types(argTypes);
        std::get<0>(proto->second)->set_return_type(returnType);
        return std::get<0>(proto->second)->codegen();
    }

    return nullptr;
}

} // namespace ast
} // namespace hannac
#endif // AST_HPP