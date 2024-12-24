#ifndef AST_HPP
#define AST_HPP

// stdlib includes
#include <cstdint>
#include <memory>
#include <vector>

namespace hannac
{
namespace ast
{
// Base class for all expressions
struct Expression
{
  public:
    Expression() = default;
    Expression(Expression const &) = default;
    Expression(Expression &&) = default;
    Expression &operator=(Expression const &) = default;
    Expression &operator=(Expression &&) = default;
    virtual ~Expression() = default;
};

/******************************************************************************
 ******************************** Variables ***********************************
 *****************************************************************************/
// Literals
struct ArrayExpression final : public Expression
{
  public:
    explicit ArrayExpression(std::vector<std::int64_t> const &arr) : mArray{arr}
    {
    }

  private:
    std::vector<std::int64_t> mArray;
};

// Variable. Currently only int64_t Arrays possible.
struct Variable final : public Expression
{
  public:
    explicit Variable(std::string const &name) : mName{name}
    {
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
        : mOperator{op}, mLHS(std::move(left)), mRHS(std::move(right))
    {
    }

  private:
    char mOperator;                   // binary operator.
    std::unique_ptr<Expression> mLHS; // Left argument.
    std::unique_ptr<Expression> mRHS; // Right argument.
};

// Function call.
struct FunctionCall final : public Expression
{
  public:
    FunctionCall(std::string const &name, std::vector<std::unique_ptr<Expression>> args)
        : mName{name}, mArguments(std::move(args))
    {
    }

  private:
    std::string mName;
    std::vector<std::unique_ptr<Expression>> mArguments;
};

// Function declaration.
struct FunctionDeclaration final : public Expression
{
  public:
    FunctionDeclaration(std::string const &name, std::vector<std::string> const &args) : mName{name}, mArguments{args}
    {
    }

    std::string get_name() const noexcept
    {
        return mName;
    }

  private:
    std::string mName;
    std::vector<std::string> mArguments;
};

// Function definition.
struct FunctionDefinition final : public Expression
{
  public:
    FunctionDefinition(std::unique_ptr<FunctionDeclaration> def, std::unique_ptr<Expression> expr)
        : mDefinition(std::move(def)), mFuncBody(std::move(expr))
    {
    }

    std::string get_name() const noexcept
    {
        return mDefinition->get_name();
    }

  private:
    std::unique_ptr<FunctionDeclaration> mDefinition;
    std::unique_ptr<Expression> mFuncBody;
};

} // namespace ast
} // namespace hannac
#endif // AST_HPP