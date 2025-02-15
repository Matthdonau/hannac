#ifndef TOKENPARSER_HPP
#define TOKENPARSER_HPP

// stdlib includes
#include <exception>
#include <map>
#include <memory>
#include <vector>

// hannac inlcudes
#include "AST.hpp"
#include "Executor.hpp"
#include "GlobalSettings.hpp"
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

/******************************************************************************
 ********************************* HELPERS ************************************
 *****************************************************************************/
inline void print_method_declaration(std::shared_ptr<ast::MethodDefinition> func)
{
    auto args = func->get_decl()->get_arguments();
    for (size_t i = 0; i < args.size(); i++)
    {
        std::cout << args[i];
        if (i < args.size() - 1)
            std::cout << ",";
        else
            std::cout << ")" << std::endl;
    }

    return;
}

inline void print_token(HTokenRes &in)
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
    case HTokenType::Return:
        std::cout << "return" << std::endl;
        break;
    case HTokenType::END:
        std::cout << "End" << std::endl;
        break;
    case HTokenType::Number:
        std::cout << std::get<std::int64_t>(in.second) << std::endl;
        break;
    case HTokenType::RealNumber:
        std::cout << std::get<double>(in.second) << std::endl;
        break;
    case HTokenType::Main:
        std::cout << "main" << std::endl;
        break;
    }
    return;
}

/******************************************************************************
 ****************************** TOKEN PARSER **********************************
 *****************************************************************************/
// Uses HLexer to parse trough hanna file.
// Returns a vector of ast::Expression which resemble the main method in correct order.
struct HTokenParser final
{
  public:
    explicit HTokenParser(HLexer &&lex) : mLexer{std::move(lex)}
    {
    }

    // Parsing main driver.
    std::vector<std::unique_ptr<ast::Expression>> parse()
    {
        // 1) Parse all method definitions.
        move_parser();
        while (mCurrentToken.first != HTokenType::Main)
        {
            switch (mCurrentToken.first)
            {
            case HTokenType::Method:
                produce_method();
                break;
            case HTokenType::END:
                throw ParseError("No main method defined in program.");
            default:
                throw ParseError{"Unkown token."};
            }
        }

        // 2) Expect main.
        if (mCurrentToken.first != HTokenType::Main)
            throw ParseError{"Expected defintion of main."};
        move_parser();

        // 3) Parse main.
        while (mCurrentToken.first != HTokenType::END)
            queue_execution();

        return std::move(mProgram);
    }

  private:
    // Move parser by one.
    inline HTokenRes move_parser()
    {
        mCurrentToken = mLexer.get_token();

        return mCurrentToken;
    }

    /******************************************************************************
     ********************************* METHODS ************************************
     *****************************************************************************/
    // Hit method keyword.
    // We now expect:
    // a) a proper declaration of the method.
    // b) a proper definition of the method.
    void produce_method()
    {
        // 1) Parse declaration of the method.
        // Expected is "method <METHOD_NAME>(<COMMA_SEPERATED_ARGUMENT_LIST>)"
        // Eat method specifier.
        move_parser();
        auto declaration = produce_declaration();

        // 2) Parse definition of the method which is basically an expression.
        // First thing to expect is a "return" since currently only one line statement methods are supported.
        if (mCurrentToken.first != HTokenType::Return)
            throw ParseError{"Non returning method: " + declaration->get_name()};
        move_parser();
        auto definition = produce_expression();
        auto func = std::make_shared<hannac::ast::MethodDefinition>(std::move(declaration), std::move(definition));
        if (HSettings::get_settings().get_verbose() > 1)
        {
            std::cout << "Produced function definition for: " << func->get_name() << "(";
            print_method_declaration(func);
        }

        // 3) Put method in method buffer.
        // We are only lazy generating code for function. That means we are only setting up the function AST node
        // here and only generate the code for it if and when it is called.
        if (ast::HMethodBuffer::get().find(func->get_name()) != ast::HMethodBuffer::get().end())
        {
            throw ParseError{"Redefinition of function " + func->get_name()};
        }
        ast::HMethodBuffer::get().insert({func->get_name(), func});

        if (HSettings::get_settings().get_verbose() > 1)
            std::cout << std::endl;

        return;
    }

    // Declaration.
    std::shared_ptr<ast::MethodDeclaration> produce_declaration()
    {
        // 1) Expect method name as very first thing after "method" keyword.
        if (mCurrentToken.first != HTokenType::Identifier)
            throw ParseError{"Expected method name."};
        // Store method name and move on to declaration.
        std::string methodName = std::get<std::string>(mCurrentToken.second);

        // 2) Next we expect opening brace "(" followed by 0-N arguments, followed by ")".
        move_parser();
        if (mCurrentToken.first != HTokenType::Character || std::get<char>(mCurrentToken.second) != '(')
            throw ParseError{"Expected '(' in method declaration of: " + methodName};

        // 3) Now we expect the function arguments.
        // Until we hit ")" we expect only Identifiers.
        std::vector<std::string> args;
        while ((move_parser()).first == HTokenType::Identifier)
        {
            args.push_back(std::get<std::string>(mCurrentToken.second));

            // Skip ','
            move_parser();
            if (mCurrentToken.first == HTokenType::Character && std::get<char>(mCurrentToken.second) != ',')
            {
                break;
            }
        }

        // 4) Now expect ')'
        if (mCurrentToken.first == HTokenType::Character)
        {
            if (char curr = std::get<char>(mCurrentToken.second) != ')')
                throw ParseError{"Expected ')' in method declaration of: " + methodName};
        }
        else
        {
            throw ParseError{"Expected ')' or variable in method declaration of: " + methodName};
        }
        // Eat ')'
        move_parser();

        return std::make_shared<ast::MethodDeclaration>(methodName, std::move(args));
    }

    /******************************************************************************
     ******************************* NUMS/VARS ************************************
     *****************************************************************************/
    std::unique_ptr<ast::Expression> produce_num()
    {
        std::unique_ptr<ast::Expression> num;
        // Create number AST.
        if (mCurrentToken.first == HTokenType::Number)
            num = std::make_unique<ast::Number>(std::get<std::int64_t>(mCurrentToken.second));
        else
            num = std::make_unique<ast::RealNumber>(std::get<double>(mCurrentToken.second));
        // Eat number and progress mCurrentToken.
        move_parser();

        return num;
    }

    std::unique_ptr<ast::Expression> produce_var(std::string name)
    {
        return std::make_unique<ast::Variable>(name);
    }

    // Identifiers.
    std::unique_ptr<ast::Expression> produce_identifier_expression()
    {
        // 1) Get name of identifier.
        auto const name = std::get<std::string>(mCurrentToken.second);

        // 2) Differentiate between variable and method call.
        move_parser();
        if (mCurrentToken.first != HTokenType::Character || std::get<char>(mCurrentToken.second) != '(')
        {
            // Variable reference.
            return produce_var(name);
        }
        else // MethodCall
        {
            std::vector<std::unique_ptr<ast::Expression>> arguments;

            // Expression.
            // Eat '('
            move_parser();
            HTokenType type = mCurrentToken.first;
            while (mCurrentToken.first != HTokenType::END)
            {
                if (type == HTokenType::Character)
                {
                    auto symbol = std::get<char>(mCurrentToken.second);
                    move_parser();
                    type = mCurrentToken.first;
                    if (symbol == ')')
                        break;

                    if (symbol == ',')
                        continue;
                }

                // Add argument.
                if (auto argument = produce_expression())
                    // Argument.
                    arguments.push_back(std::move(argument));
                else
                    return nullptr;

                type = mCurrentToken.first;
            }
            return std::make_unique<ast::MethodCall>(name, std::move(arguments));
        }
    }

    /******************************************************************************
     ******************************** EXECUTION ************************************
     *****************************************************************************/
    void queue_execution()
    {
        // Produce expression.
        // Will result in either a ast::MethodCall or ast::Binary.
        auto expr = produce_expression();

        // Add to program to be executed.
        mProgram.push_back(std::move(expr));

        return;
    }

    std::unique_ptr<ast::Expression> produce_expression()
    {
        // We expect some expression: This could be:
        // 1) a method call.
        // 2) a binary expression.
        auto first = parse_statement();

        // In case of a function call return the method call.
        if (first->get_type() == ast::ASTType::MethodCall)
            return first;
        else if (mCurrentToken.first == HTokenType::Character &&
                 mOpPrecedence.find(std::get<char>(mCurrentToken.second)) == mOpPrecedence.end())
            return first;
        else
            // We got a binary expression.
            // Handle this by recursively calling produce_expression.
            return parse_binary_op_rhs(0, std::move(first));
    }

    std::unique_ptr<ast::Expression> parse_statement()
    {
        switch (mCurrentToken.first)
        {
        case HTokenType::Identifier:
            return produce_identifier_expression();
        case HTokenType::Number:
        case HTokenType::RealNumber:
            return produce_num();
        default:
            throw ParseError{"Unknown character while expecting expression statement."};
        }
    }

    std::unique_ptr<ast::Expression> parse_binary_op_rhs(int expr_precedence, std::unique_ptr<ast::Expression> LHS)
    {
        // Parse trough expression until we find no RHS anymore.
        while (true)
        {
            // 1) Get precedence of current binary operator.
            int prec = -1;
            if (mCurrentToken.first == HTokenType::Character)
            {
                char op = std::get<char>(mCurrentToken.second);
                if (mOpPrecedence.find(op) != mOpPrecedence.end())
                    prec = mOpPrecedence[op];
            }
            // If not binary operator or operator with less precedence, return.
            if (prec < expr_precedence)
                return LHS;

            // Save binary operator.
            char binOp = std::get<char>(mCurrentToken.second);

            // 2) Parse right hand side.
            move_parser();
            auto RHS = parse_statement();
            if (!RHS)
                return nullptr;

            // 3) Get precedence of right hand side.
            int nextPrec = -1;
            if (mCurrentToken.first == HTokenType::Character)
            {
                char op = std::get<char>(mCurrentToken.second);
                if (mOpPrecedence.find(op) != mOpPrecedence.end())
                    nextPrec = mOpPrecedence[op];
            }

            // 4) Decide which way to go.
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

    HLexer mLexer;
    HTokenRes mCurrentToken;
    std::map<char, int> mOpPrecedence{{'+', 20}, {'-', 20}, {'*', 40}, {'/', 40}};
    std::vector<std::unique_ptr<ast::Expression>> mProgram;
};
} // namespace hannac
#endif // TOKENPARSER_HPP