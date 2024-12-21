#include "gtest/gtest.h"

#include "FileParser.hpp"

// stdlib includes
#include <stdio.h>
#include <filesystem>
#include <string>

TEST(HFileParser, NoFile)
{
    bool exception{false};
    try
    {
        hannac::HFileParser("");
    }
    catch(const std::exception& e)
    {
        exception = true;
    }
    EXPECT_EQ(exception, true);
   
}

TEST(HFileParser, NonExistingFile)
{
    bool exception{false};
    try
    {
        hannac::HFileParser("Foo.hanna");
    }
    catch(const std::exception& e)
    {
        exception = true;
    }
    EXPECT_EQ(exception, true);
}

TEST(HFileParser, WrongFileExtension)
{
    bool exception{false};
    try
    {
        hannac::HFileParser("Foo.hann");
    }
    catch(const std::exception& e)
    {
        exception = true;
    }
    EXPECT_EQ(exception, true);
}

TEST(HFileParser, Correct)
{
    bool exception{false};
    try
    {
        std::filesystem::path path(__FILE__);
        hannac::HFileParser Parser(path.parent_path().string() + "/" + "Bar.hanna");
        std::string fileContent;
        char current;
        while((current=Parser.read()) != EOF)
            fileContent+=current;

        EXPECT_EQ(fileContent, std::string{"# Test\n# Test"});
    }
    catch(const std::exception& e)
    {
        exception = true;
    }
    EXPECT_EQ(exception, false);

}