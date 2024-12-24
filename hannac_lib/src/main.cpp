// stdlib includes
#include <exception>
#include <iostream>
#include <stdio.h>

// hannac includes
#include "AST.hpp"
#include "FileParser.hpp"
#include "Lexer.hpp"
#include "TokenParser.hpp"

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
        hannac::HTokenParser parser{hannac::HLexer{hannac::HFileParser{filename}}};
        parser.run();
    }
    catch (const std::exception &excep)
    {
        std::cerr << excep.what() << '\n';
    }

    return 1;
}