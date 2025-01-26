#include "Executor.hpp"
#include "FileParser.hpp"
#include "TokenParser.hpp"
#include "gtest/gtest.h"

// stdlib includes
#include <filesystem>
#include <iostream>
#include <string>

TEST(HTokenParser, MissingReturn)
{
    std::filesystem::path path(__FILE__);
    hannac::HTokenParser parser{
        hannac::HLexer{hannac::HFileParser{path.parent_path().string() + "/data/" + "missingReturn.hanna"}}};

    bool exception = false;
    try
    {
        hannac::HExecutor ex{parser.parse()};
    }
    catch (const std::exception &e)
    {
        exception = true;
    }
    ASSERT_EQ(exception, true);
}


TEST(HTokenParser, NoMain)
{
    std::filesystem::path path(__FILE__);
    hannac::HTokenParser parser{
        hannac::HLexer{hannac::HFileParser{path.parent_path().string() + "/data/" + "noMain.hanna"}}};

    bool exception = false;
    try
    {
        hannac::HExecutor ex{parser.parse()};
    }
    catch (const std::exception &e)
    {
        exception = true;
    }
    ASSERT_EQ(exception, true);
}