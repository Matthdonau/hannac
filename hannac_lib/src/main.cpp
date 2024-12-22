// stdlib includes
#include <exception>
#include <iostream>
#include <stdio.h>

// hannac includes
#include "FileParser.hpp"
#include "Lexer.hpp"

int main(int argc, char *argv[])
{
    if (argc != 2) // Expect only filepath+filename as input.
        return 0;

    // Get filename
    auto const &filename = argv[1];
    std::cout << "Compiling: " << filename << std::endl;

    // Start compiler.
    try
    {
        // Setup parsing of hanna file.
        hannac::HLexer lexer{hannac::HFileParser{filename}};
        auto currentToken = lexer.get_token();
        while (currentToken.first != hannac::HTokenType::END)
        {
            std::cout << "New Token:" << std::endl;
            if (currentToken.first == hannac::HTokenType::Identifier ||
                currentToken.first == hannac::HTokenType::Method)
                std::cout << std::get<std::string>(currentToken.second) << std::endl;
            else if (currentToken.first == hannac::HTokenType::NumArray)
                for (auto const &el : std::get<hannac::NumArray>(currentToken.second))
                    std::cout << el << std::endl;

            currentToken = lexer.get_token();
        }
    }
    catch (const std::exception &excep)
    {
        std::cerr << excep.what() << '\n';
    }

    return 1;
}