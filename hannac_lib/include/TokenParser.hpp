#ifndef TOKENPARSER_HPP
#define TOKENPARSER_HPP

// stdlib includes
#include <exception>
#include <map>
#include <memory>
#include <vector>

// hannac inlcudes
#include "AST.hpp"
#include "Lexer.hpp"

namespace hannac
{
struct ParseError : public std::exception
{
  public:
    ParseError(std::string const &message) : mMessage{message}
    {
    }

    const char *what() const throw()
    {
        return mMessage.c_str();
    }

  private:
    std::string mMessage;
};

void print_token(HTokenRes &in)
{
    switch (in.first)
    {
    case HTokenType::Character:
        std::cout << std::get<char>(in.second) << std::endl;
        break;
    case HTokenType::Identifier:
        std::cout << std::get<std::string>(in.second) << std::endl;
        break;
    case HTokenType::Method:
        std::cout << std::get<std::string>(in.second) << std::endl;
        break;
    case HTokenType::END:
        std::cout << "End" << std::endl;
        break;
    case HTokenType::NumArray:
        for (auto const &el : std::get<NumArray>(in.second))
            std::cout << el << std::endl;
        break;
    }

    return;
}

struct HTokenParser final
{
  public:
    explicit HTokenParser(HLexer &&lex) : mLexer{std::move(lex)}
    {
    }

    // Parsing main driver.
    void run()
    {
        mCurrentToken = mLexer.get_token();
        while (mCurrentToken.first != HTokenType::END)
        {
            std::cout << std::endl << "Next token: ";
            print_token(mCurrentToken);
            switch (mCurrentToken.first)
            {
            case HTokenType::Method:
                method();
                break;
            case HTokenType::Character:
                break;
            default:
                top_level();
            }
        }
        return;
    }

  private:
    std::unique_ptr<ast::Expression> parse_primary()
    {
        switch (mCurrentToken.first)
        {
            std::cout << "Primary: ";
            print_token(mCurrentToken);
        case HTokenType::Identifier:
            return produce_identifier_expression();
        case HTokenType::NumArray:
            return produce_array();
        default:
            // std::cout << "Unknown character while expecting expression." << std::endl;
            return nullptr;
        }
    }

    // Wrappers
    void method()
    {
        if (auto func = produce_defintion())
        {
            std::cout << "Produced function definition for: " << func->get_name() << std::endl;
        }
        else
        {
            std::cout << "Error while parsing function." << std::endl;
        }

        return;
    }

    void top_level()
    {
        if (auto func = produce_top_level())
        {
            std::cout << "Produced top level expression." << std::endl;
        }
        else
        {
            std::cout << "Error while parsing top level expression." << std::endl;
        }

        return;
    }

    // Top level.
    std::unique_ptr<ast::Expression> produce_top_level()
    {
        if (auto expr = produce_expression())
        {
            auto declaration = std::make_unique<ast::FunctionDeclaration>("", std::vector<std::string>());
            return std::make_unique<ast::FunctionDefinition>(std::move(declaration), std::move(expr));
        }
        return nullptr;
    }

    // Arrays.
    std::unique_ptr<ast::Expression> produce_array()
    {
        // Progress mCurrentToken.
        auto arr = std::make_unique<ast::ArrayExpression>(std::get<NumArray>(mCurrentToken.second));
        // Eat arr.
        mCurrentToken = mLexer.get_token();
        return arr;
    }

    // Declaration.
    std::unique_ptr<ast::FunctionDeclaration> produce_declaration()
    {
        if (mCurrentToken.first != HTokenType::Identifier)
        {
            std::cout << "No function name in method declaration." << std::endl;
            return nullptr;
        }
        // Get function name and move on to declaration.
        std::string methodName = std::get<std::string>(mCurrentToken.second);
        std::cout << "Parsing function: " << methodName << std::endl;

        // Next we expect opening brace "(".
        mCurrentToken = mLexer.get_token();
        if (mCurrentToken.first == HTokenType::Character)
        {
            char curr = std::get<char>(mCurrentToken.second);
            if (curr != '(')
            {
                std::cout << "Expected '(' in method declaration,"
                          << "but got: " << std::get<char>(mCurrentToken.second) << std::endl;
                return nullptr;
            }
        }
        else
        {
            std::cout << "Expected '(' in method declaration." << std::endl;
            return nullptr;
        }

        // Now we expect the function arguments.
        std::vector<std::string> args;
        while ((mCurrentToken = mLexer.get_token()).first == HTokenType::Identifier)
        {
            std::cout << "Adding function parameter: " << std::get<std::string>(mCurrentToken.second) << std::endl;
            args.push_back(std::get<std::string>(mCurrentToken.second));

            // Skip ','
            mCurrentToken = mLexer.get_token();
            if (mCurrentToken.first == HTokenType::Character && std::get<char>(mCurrentToken.second) != ',')
            {
                break;
            }
        }

        // Now expect ')'
        if (mCurrentToken.first == HTokenType::Character)
        {
            char curr = std::get<char>(mCurrentToken.second);
            if (curr != ')')
            {
                std::cout << "Expected ')' in method declaration" << std::endl;
                return nullptr;
            }
        }
        else
        {
            std::cout << "Expected ')' in method declaration" << std::endl;
            return nullptr;
        }

        // Eat ')'
        mCurrentToken = mLexer.get_token();

        return std::make_unique<ast::FunctionDeclaration>(methodName, std::move(args));
    }

    // Defitinion.
    std::unique_ptr<ast::FunctionDefinition> produce_defintion()
    {
        // Eat method specifier.
        mCurrentToken = mLexer.get_token();

        // Get funcion definition.
        auto declaration = produce_declaration();
        if (declaration == nullptr)
        {
            std::cout << "Error parsing declaration of method: " << std::endl;
            return nullptr;
        }

        auto definition = produce_expression();
        if (definition == nullptr)
        {
            // Wrong things happened.
            std::cout << "Error parsing definition of method: " << declaration->get_name() << std::endl;
            return nullptr;
        }

        return std::make_unique<ast::FunctionDefinition>(std::move(declaration), std::move(definition));
    }

    // Identifiers.
    std::unique_ptr<ast::Expression> produce_identifier_expression()
    {
        // Passed token is Identifier string.
        auto const name = std::get<std::string>(mCurrentToken.second);
        std::vector<std::unique_ptr<ast::Expression>> arguments;

        // Get next token.
        mCurrentToken = mLexer.get_token();
        if (mCurrentToken.first != HTokenType::Character || std::get<char>(mCurrentToken.second) != '(')
        {
            // Variable reference.
            return std::make_unique<ast::Variable>(name);
        }
        else
        {
            // Expression.
            // Eat '('
            mCurrentToken = mLexer.get_token();
            HTokenType type = mCurrentToken.first;
            while (mCurrentToken.first != HTokenType::END)
            {
                if (auto argument = produce_expression())
                {
                    // Argument.
                    arguments.push_back(std::move(argument));
                }
                else
                    return nullptr;

                type = mCurrentToken.first;
                if (type == HTokenType::Character && std::get<char>(mCurrentToken.second) == ')')
                {
                    break;
                }
                else if (type == HTokenType::Character && std::get<char>(mCurrentToken.second) == ',')
                {
                    // Skip argument seperator
                    mCurrentToken = mLexer.get_token();
                    continue;
                }

                mCurrentToken = mLexer.get_token();
                type = mCurrentToken.first;
            }

            // Eat ')'
            mCurrentToken = mLexer.get_token();
        }
        auto call = std::make_unique<ast::FunctionCall>(name, std::move(arguments));
        call->print_args();
        return call;
    }

    // Expression stuff.
    int get_op_precedence(char op)
    {
        if (!isascii(op))
            return -1;

        // Check if defined in valid binary operators.
        if (mOpPrecedence.find(op) == mOpPrecedence.end())
            return -1;

        return mOpPrecedence[op];
    }

    std::unique_ptr<ast::Expression> parse_binary_op_rhs(int expr_precedence, std::unique_ptr<ast::Expression> LHS)
    {
        while (true)
        {
            int prec = -1;
            if (mCurrentToken.first == HTokenType::Character)
                prec = get_op_precedence(std::get<char>(mCurrentToken.second));

            if (prec < expr_precedence)
                return LHS;

            // Save binary operator.
            char binOp = std::get<char>(mCurrentToken.second);
            mCurrentToken = mLexer.get_token();

            auto RHS = parse_primary();
            if (!RHS)
                return nullptr;

            int nextPrec = -1;
            if (mCurrentToken.first == HTokenType::Character)
                nextPrec = get_op_precedence(std::get<char>(mCurrentToken.second));

            if (prec < nextPrec)
            {
                RHS = parse_binary_op_rhs(prec + 1, std::move(RHS));
                if (!RHS)
                    return nullptr;
            }

            // Merge LHS/RHS
            LHS = std::make_unique<ast::Binary>(binOp, std::move(LHS), std::move(RHS));
        }
    }

    std::unique_ptr<ast::Expression> produce_expression()
    {
        auto LHS = parse_primary();
        if (!LHS)
            return nullptr;

        return parse_binary_op_rhs(0, std::move(LHS));
    }

    HLexer mLexer;
    HTokenRes mCurrentToken;
    std::map<char, int> mOpPrecedence{{'<', 10}, {'+', 20}, {'-', 20}, {'*', 40}};
};
} // namespace hannac
#endif // TOKENPARSER_HPP