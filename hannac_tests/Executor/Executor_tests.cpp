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

TEST(HExecutor, FunctionCall)
{
    std::filesystem::path path(__FILE__);
    hannac::HTokenParser parser{
        hannac::HLexer{hannac::HFileParser{path.parent_path().string() + "/data/" + "functionCall.hanna"}}};

    hannac::HExecutor ex{parser.parse()};
    auto results{ex()};
    EXPECT_EQ(results.size(), 5);

    // 10-5
    EXPECT_EQ(hannac::HResultType::INT, results[0].get_type());
    EXPECT_EQ(5, results[0].get_result().i);
    // 5-10
    EXPECT_EQ(hannac::HResultType::INT, results[1].get_type());
    EXPECT_EQ(-5, results[1].get_result().i);

    // 10.5 - 5.4
    EXPECT_EQ(hannac::HResultType::REAL, results[2].get_type());
    EXPECT_EQ(5.1, results[2].get_result().r);
    // 5.4 - 10.5
    EXPECT_EQ(hannac::HResultType::REAL, results[3].get_type());
    EXPECT_EQ(-5.1, results[3].get_result().r);

    EXPECT_EQ(hannac::HResultType::INT, results[4].get_type());
    EXPECT_EQ(55, results[4].get_result().i);
}

TEST(HExecutor, ParameterOrder)
{
    std::filesystem::path path(__FILE__);
    hannac::HTokenParser parser{
        hannac::HLexer{hannac::HFileParser{path.parent_path().string() + "/data/" + "parameterOrder.hanna"}}};

    hannac::HExecutor ex{parser.parse()};
    auto results{ex()};
    EXPECT_EQ(results.size(), 4);

    // 10/5
    EXPECT_EQ(hannac::HResultType::INT, results[0].get_type());
    EXPECT_EQ(2, results[0].get_result().i);
    // 5/10
    EXPECT_EQ(hannac::HResultType::INT, results[1].get_type());
    EXPECT_EQ(0, results[1].get_result().i);

    // 5/10
    EXPECT_EQ(hannac::HResultType::INT, results[2].get_type());
    EXPECT_EQ(0, results[2].get_result().i);
    // 10/5
    EXPECT_EQ(hannac::HResultType::INT, results[3].get_type());
    EXPECT_EQ(2, results[3].get_result().i);
}

TEST(HExecutor, ExprAsParameter)
{
    std::filesystem::path path(__FILE__);
    hannac::HTokenParser parser{
        hannac::HLexer{hannac::HFileParser{path.parent_path().string() + "/data/" + "expressionAsParameter.hanna"}}};

    hannac::HExecutor ex{parser.parse()};
    auto results{ex()};
    EXPECT_EQ(results.size(), 5);

    EXPECT_EQ(hannac::HResultType::INT, results[0].get_type());
    EXPECT_EQ(100, results[0].get_result().i);

    EXPECT_EQ(hannac::HResultType::REAL, results[1].get_type());
    EXPECT_EQ(117.6, results[1].get_result().r);

    EXPECT_EQ(hannac::HResultType::INT, results[2].get_type());
    EXPECT_EQ(112, results[2].get_result().i);

    EXPECT_EQ(hannac::HResultType::REAL, results[3].get_type());
    EXPECT_EQ(2.2, results[3].get_result().r);

    EXPECT_EQ(hannac::HResultType::INT, results[4].get_type());
    EXPECT_EQ(16, results[4].get_result().i);
}