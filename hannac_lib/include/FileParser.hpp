#ifndef FILEPARSER_HPP
#define FILEPARSER_HPP

// stdlibincludes
#include <string>
#include <filesystem>
#include <exception>
#include <fstream>
#include <stdio.h>

//DEBUG
#include <iostream>

namespace hannac
{
struct FileError : public std::exception
{
    public:
    FileError(std::string const& message)
    : mMessage{message}
    {}

    const char* what() const throw()
    {
        return mMessage.c_str();
    }

    private:
    std::string mMessage;
};

struct HFileParser final
{
    public:
    HFileParser(std::filesystem::path const& sourceFilePath)
    : mSourceFilePath{sourceFilePath}
    {
        auto const& extension = mSourceFilePath.extension();
        
        // Check for correct file type.
        if(extension != ".hanna")
            throw FileError{"Wrong file extension!"};
        
        // Open file.
        mFile = std::ifstream(mSourceFilePath.string(), std::ios::binary);
        if(!mFile.is_open())
            throw FileError{"Unable to open source file for reading."};
        mFile >> std::noskipws; // Don't skip whitespaces
    }

    // Read next character.
    char read()
    {
        // If at end of file indicate so.
        if(mFile.eof())
            return EOF;

        // Get next char,
        char current;
        mFile >> current;

        // We have to check again here for EOF.
        if(mFile.eof())
            return EOF;

        return current;
    };

    private:
    std::filesystem::path mSourceFilePath;
    std::ifstream mFile;
};
} // namespace hannac
#endif // FILEPARSER_HPP