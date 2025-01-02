#include "Executor.hpp"
#include "FileParser.hpp"
#include "TokenParser.hpp"
#include "gtest/gtest.h"

// stdlib includes
#include <filesystem>
#include <iostream>
#include <string>

TEST(HExecutor, RealMethod)
{
    std::filesystem::path path(__FILE__);
    hannac::HTokenParser parser{
        hannac::HLexer{hannac::HFileParser{path.parent_path().string() + "/data/" + "real.hanna"}}};

    hannac::HExecutor ex{parser.parse()};
    auto results{ex()};
    EXPECT_EQ(results.size(), 2);

    EXPECT_EQ(hannac::HResultType::REAL, results[0].get_type());
    EXPECT_EQ(1.5, results[0].get_result().r);

    EXPECT_EQ(hannac::HResultType::REAL, results[1].get_type());
    EXPECT_EQ(1.15, results[1].get_result().r);
}

TEST(HExecutor, IntMethod)
{
    std::filesystem::path path(__FILE__);
    hannac::HTokenParser parser{
        hannac::HLexer{hannac::HFileParser{path.parent_path().string() + "/data/" + "int.hanna"}}};

    hannac::HExecutor ex{parser.parse()};
    auto results{ex()};
    EXPECT_EQ(results.size(), 2);

    EXPECT_EQ(hannac::HResultType::INT, results[0].get_type());
    EXPECT_EQ(5, results[0].get_result().i);

    EXPECT_EQ(hannac::HResultType::INT, results[1].get_type());
    EXPECT_EQ(10150, results[1].get_result().i);
}

TEST(HExecutor, Both)
{
    std::filesystem::path path(__FILE__);
    hannac::HTokenParser parser{
        hannac::HLexer{hannac::HFileParser{path.parent_path().string() + "/data/" + "both.hanna"}}};

    hannac::HExecutor ex{parser.parse()};
    auto results{ex()};
    EXPECT_EQ(results.size(), 2);

    EXPECT_EQ(hannac::HResultType::INT, results[0].get_type());
    EXPECT_EQ(5, results[0].get_result().i);

    EXPECT_EQ(hannac::HResultType::REAL, results[1].get_type());
    EXPECT_EQ(21.5, results[1].get_result().r);
}
