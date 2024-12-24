#ifndef TOKENPARSER_HPP
#define TOKENPARSER_HPP

// stdlib includes
#include <exception>
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

struct HTokenParser final
{
  public:
    explicit HTokenParser(HLexer &&lex) : mLexer{std::move(lex)}
    {
    }

    // Parsing main driver.
    void run()
    {
        while ((mCurrentToken = mLexer.get_token()).first != HTokenType::END)
        {
            switch (mCurrentToken.first)
            {
            case HTokenType::Method:
                method();
                break;
            case HTokenType::Character:
                break;
            default:
                top_level();
                break;
            }
        }
        return;
    }

  private:
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
        return;
    }

    // Arrays.
    std::unique_ptr<ast::Expression> produce_array(HToken const &token)
    {
        return std::make_unique<ast::ArrayExpression>(std::get<NumArray>(token));
    }

    // Declaration.
    std::unique_ptr<ast::FunctionDeclaration> produce_declaration()
    {
        if (mCurrentToken.first != HTokenType::Identifier)
        {
            std::cout << "No function name in method declaration." << std::endl;
            return nullptr;
        }
        std::cout << "Parsing function: " << std::get<std::string>(mCurrentToken.second) << std::endl;

        // Get function name and move on to declaration.
        std::string methodName = std::get<std::string>(mCurrentToken.second);
        mCurrentToken = mLexer.get_token();

        // Next we expect opening brace "(".
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
            args.push_back(std::get<std::string>(mCurrentToken.second));

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
            return nullptr;

        auto definition = produce_expression();
        if (definition == nullptr)
            // Wrong things happened.
            return nullptr;

        return std::make_unique<ast::FunctionDefinition>(std::move(declaration), std::move(definition));
    }

    // Identifiers.
    std::unique_ptr<ast::Expression> produce_identifier(HToken const &token)
    {
        // Passed token is Identifier string.
        auto const name = std::get<std::string>(token);
        std::vector<std::unique_ptr<ast::Expression>> arguments;

        // Get next token.
        mCurrentToken = mLexer.get_token();

        if (mCurrentToken.first == HTokenType::Character)
        {
            if (std::get<char>(mCurrentToken.second) != '(')
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
                while (true)
                {
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
                    else
                    {
                        // Argument.
                        arguments.push_back(produce_expression());
                    }

                    mCurrentToken = mLexer.get_token();
                    type = mCurrentToken.first;
                }

                // Eat ')'

                mCurrentToken = mLexer.get_token();
            }
        }
        else
        {
            throw ParseError{"Expected EOL or ("};
        }

        return std::make_unique<ast::FunctionCall>(name, std::move(arguments));
    }

    // Expression stuff.
    std::unique_ptr<ast::Expression> produce_expression()
    {
        return nullptr;
    }

    HLexer mLexer;
    HTokenRes mCurrentToken;
};
} // namespace hannac
#endif // TOKENPARSER_HPP