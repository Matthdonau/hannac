// stdlib includes
#include <iostream>
#include <exception>
#include <stdio.h>

// hannac includes
#include "FileParser.hpp"

int main(int argc, char* argv[])
{
    if(argc != 2) // Expect only filepath+filename as input.
        return 0;

    // Get filename
    auto const& filename = argv[1];
    std::cout << "Compiling: " << filename << std::endl;
    
    // Start compiler.
    try
    {
        // Setup parsing of hanna file.
        hannac::HFileParser Parser(filename);
        char current;
        while ((current = Parser.read()) != EOF)
        {
            std::cout << current;
        }
        std::cout << std::endl;


    }
    catch(const std::exception& excep)
    {
        std::cerr << excep.what() << '\n';
    }

    return 1;
}