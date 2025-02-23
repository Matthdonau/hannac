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

TEST(HExecutor, MethodAsParam)
{
    std::filesystem::path path(__FILE__);
    hannac::HTokenParser parser{
        hannac::HLexer{hannac::HFileParser{path.parent_path().string() + "/data/" + "methodAsParam.hanna"}}};

    hannac::HExecutor ex{parser.parse()};
    auto results{ex()};
    EXPECT_EQ(results.size(), 2);

    EXPECT_EQ(hannac::HResultType::INT, results[0].get_type());
    EXPECT_EQ(16, results[0].get_result().i);

    EXPECT_EQ(hannac::HResultType::INT, results[1].get_type());
    EXPECT_EQ(15, results[1].get_result().i);
}

TEST(HExecutor, Negative)
{
    // hannac::HSettings::get_settings().set_verbose(2);
    std::filesystem::path path(__FILE__);
    hannac::HTokenParser parser{
        hannac::HLexer{hannac::HFileParser{path.parent_path().string() + "/data/" + "negative.hanna"}}};

    hannac::HExecutor ex{parser.parse()};
    auto results{ex()};
    EXPECT_EQ(results.size(), 28);

    // -7 + 0
    EXPECT_EQ(hannac::HResultType::INT, results[0].get_type());
    EXPECT_EQ(-7, results[0].get_result().i);

    // 7 + 0
    EXPECT_EQ(hannac::HResultType::INT, results[1].get_type());
    EXPECT_EQ(7, results[1].get_result().i);

    // -7+  7
    EXPECT_EQ(hannac::HResultType::INT, results[2].get_type());
    EXPECT_EQ(0, results[2].get_result().i);

    // 7.1 + 0.1
    EXPECT_EQ(hannac::HResultType::REAL, results[3].get_type());
    EXPECT_FLOAT_EQ(7.2, results[3].get_result().r);

    // -7.1 + 0.1
    EXPECT_EQ(hannac::HResultType::REAL, results[4].get_type());
    EXPECT_FLOAT_EQ(-7.0, results[4].get_result().r);

    // -7.1 + 7.1
    EXPECT_EQ(hannac::HResultType::REAL, results[5].get_type());
    EXPECT_FLOAT_EQ(0.0, results[5].get_result().r);

    // -7.1 - 7.1
    EXPECT_EQ(hannac::HResultType::REAL, results[6].get_type());
    EXPECT_FLOAT_EQ(-14.2, results[6].get_result().r);

    // -7 -14
    EXPECT_EQ(hannac::HResultType::INT, results[7].get_type());
    EXPECT_EQ(-21, results[7].get_result().i);

    // -1 + -1
    EXPECT_EQ(hannac::HResultType::INT, results[8].get_type());
    EXPECT_EQ(-2, results[8].get_result().i);

    // -1 + 1
    EXPECT_EQ(hannac::HResultType::INT, results[9].get_type());
    EXPECT_EQ(0, results[9].get_result().i);

    // 1 + -1
    EXPECT_EQ(hannac::HResultType::INT, results[10].get_type());
    EXPECT_EQ(0, results[10].get_result().i);

    // -0 + -0
    EXPECT_EQ(hannac::HResultType::INT, results[11].get_type());
    EXPECT_EQ(0, results[11].get_result().i);

    // -128 - 128
    EXPECT_EQ(hannac::HResultType::INT, results[12].get_type());
    EXPECT_EQ(-256, results[12].get_result().i);

    // 128 - -128
    EXPECT_EQ(hannac::HResultType::INT, results[13].get_type());
    EXPECT_EQ(256, results[13].get_result().i);

    // -128 - -128
    EXPECT_EQ(hannac::HResultType::INT, results[14].get_type());
    EXPECT_EQ(0, results[14].get_result().i);

    // -1.4 + -1.3
    EXPECT_EQ(hannac::HResultType::REAL, results[15].get_type());
    EXPECT_FLOAT_EQ(-2.7, results[15].get_result().r);

    // -1.4 + 1.3
    EXPECT_EQ(hannac::HResultType::REAL, results[16].get_type());
    EXPECT_FLOAT_EQ(-0.1, results[16].get_result().r);

    // 1.4 + -1.7
    EXPECT_EQ(hannac::HResultType::REAL, results[17].get_type());
    EXPECT_FLOAT_EQ(-0.3, results[17].get_result().r);

    // -0.0 + -0.0
    EXPECT_EQ(hannac::HResultType::REAL, results[18].get_type());
    EXPECT_FLOAT_EQ(0.0, results[18].get_result().r);

    // -128.5 - 128.6
    EXPECT_EQ(hannac::HResultType::REAL, results[19].get_type());
    EXPECT_FLOAT_EQ(-257.1, results[19].get_result().r);

    // 128.7 - -128.7
    EXPECT_EQ(hannac::HResultType::REAL, results[20].get_type());
    EXPECT_FLOAT_EQ(257.4, results[20].get_result().r);

    // -128.3 - -128.3
    EXPECT_EQ(hannac::HResultType::REAL, results[21].get_type());
    EXPECT_FLOAT_EQ(0.0, results[21].get_result().r);

    // -128.3 - 128.3
    EXPECT_EQ(hannac::HResultType::REAL, results[22].get_type());
    EXPECT_FLOAT_EQ(-256.6, results[22].get_result().r);

    // 7 - -7
    EXPECT_EQ(hannac::HResultType::INT, results[23].get_type());
    EXPECT_EQ(14, results[23].get_result().i);

    // -7 - -7
    EXPECT_EQ(hannac::HResultType::INT, results[24].get_type());
    EXPECT_EQ(0, results[24].get_result().i);

    // -7 + -7
    EXPECT_EQ(hannac::HResultType::INT, results[25].get_type());
    EXPECT_EQ(-14, results[25].get_result().i);

    // -7 - +7
    EXPECT_EQ(hannac::HResultType::INT, results[26].get_type());
    EXPECT_EQ(-14, results[26].get_result().i);

    // 7.1 - -7.2
    EXPECT_EQ(hannac::HResultType::REAL, results[27].get_type());
    EXPECT_FLOAT_EQ(14.3, results[27].get_result().r);
}