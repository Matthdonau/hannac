// stdlib includes
#include <exception>
#include <iostream>
#include <memory>
#include <stdio.h>

// hannac includes
#include "AST.hpp"
#include "Codegen.hpp"
#include "FileParser.hpp"
#include "GlobalSettings.hpp"
#include "Lexer.hpp"
#include "TokenParser.hpp"

#define HANNAC_VERSION "0.0.1"

void print_help()
{
    std::cout << "Hannac compiler/interpreter." << "(" << HANNAC_VERSION << ")" << std::endl;
    std::cout << "Usage: hannac <HANNA_FILE> <COMMAND_LINE_OPTIONS>" << std::endl;
    std::cout << "Command line options:" << std::endl;
    std::cout << "-v,--verbose:\t" << "Enable verbose logging." << std::endl;
    std::cout << "-h,--help:\t" << "Print this text" << std::endl;
    std::cout << "--version:\t" << "Print version" << std::endl;

    return;
}
int main(int argc, char *argv[])
{
    // Parse and check arguments.
    // First argument needs to be hanna file.
    if (argc < 2)
    {
        std::cout << "No hanna file provided." << std::endl;
        return 0;
    }
    // Parse command line arguments.
    std::string filename{};
    for (int i = 1; i < argc; i++)
    {
        std::string arg{argv[i]};
        if (arg[0] != '-')
        {
            // Get filename
            filename = arg;
        }
        else if (arg == "-v" || arg == "--verbose")
        {
            hannac::HSettings::get_settings().set_verbose();
        }
        else if (arg == "-h" || arg == "--help")
        {
            print_help();
            return 1;
        }
        else if (arg == "--version")
        {
            std::cout << HANNAC_VERSION << std::endl;
            return 1;
        }
        else
        {
            std::cout << "Unkown argument: " << arg << std::endl;
            return 0;
        }
    }
    std::cout << "Compiling: " << filename << std::endl;

    // Start compiler.
    try
    {
        // Setup parsing of hanna file.
        hannac::HTokenParser parser{hannac::HLexer{hannac::HFileParser{filename}}};

        // Parse program.
        // Execute program.
        hannac::HExecutor ex{parser.parse()};
        ex();
    }
    catch (const std::exception &excep)
    {
        const std::string red("\033[0;31m");
        std::string reset("\033[0m");
        std::cerr << red << "ERROR: " << excep.what() << reset << std::endl;
    }

    return 1;
}