#ifndef AST_HPP
#define AST_HPP

// stdlib includes
#include <cstdint>
#include <vector>

namespace hannac
{
namespace ast
{
// Base class for all expresions
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

// Literals
struct ArrayExpression final : public Expression
{
  public:
    ArrayExpression(std::vector<std::int64_t> arr) : mArray{arr}
    {
    }

  private:
    std::vector<std::int64_t> mArray;
};

} // namespace ast
} // namespace hannac
#endif // AST_HPP