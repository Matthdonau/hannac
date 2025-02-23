#include "FileParser.hpp"
#include "Lexer.hpp"
#include "gtest/gtest.h"

// stdlib includes
#include <stdio.h>

#include <cstdint>
#include <filesystem>
#include <string>
#include <utility>
#include <vector>

hannac::HTokenRes move_parser_helper(hannac::HLexer &lexer)
{
    auto currentToken = lexer.get_token();
    while (currentToken.first == hannac::HTokenType::EOL)
        currentToken = lexer.get_token();

    return currentToken;
}

TEST(HLexer, Comments)
{
    std::filesystem::path path(__FILE__);
    hannac::HLexer lexer{hannac::HFileParser{path.parent_path().string() + "/data/" + "comment.hanna"}};

    std::vector<hannac::HToken> tokens;

    hannac::HTokenRes current{};
    while ((current = move_parser_helper(lexer)).first != hannac::HTokenType::END)
    {
        tokens.push_back(current.second);
    }

    EXPECT_EQ(tokens.size(), 0);
}

TEST(HLexer, IntNumbers)
{
    std::filesystem::path path(__FILE__);
    hannac::HLexer lexer{hannac::HFileParser{path.parent_path().string() + "/data/" + "int_numbers.hanna"}};

    std::vector<hannac::HToken> tokens;

    hannac::HTokenRes current{};
    while ((current = move_parser_helper(lexer)).first != hannac::HTokenType::END)
    {
        EXPECT_EQ(hannac::HTokenType::Number, current.first);
        tokens.push_back(current.second);
    }

    EXPECT_EQ(tokens.size(), 4);
    std::vector<std::int64_t> expected{123, 456, 999, 789};
    int i = 0;
    for (auto const &token : tokens)
    {
        EXPECT_EQ(expected[i], std::get<std::int64_t>(token));
        i++;
    }
}

TEST(HLexer, RealNumbers)
{
    std::filesystem::path path(__FILE__);
    hannac::HLexer lexer{hannac::HFileParser{path.parent_path().string() + "/data/" + "real_numbers.hanna"}};

    std::vector<hannac::HToken> tokens;

    hannac::HTokenRes current{};
    while ((current = move_parser_helper(lexer)).first != hannac::HTokenType::END)
    {
        tokens.push_back(current.second);
    }

    EXPECT_EQ(tokens.size(), 4);
    std::vector<double> expected{1.0, 99.9, 999999999.9, 1234.5678};
    int i = 0;
    for (auto const &token : tokens)
    {
        EXPECT_EQ(expected[i], std::get<double>(token));
        i++;
    }
}

TEST(HLexer, RealNumbersIncorrect)
{
    std::filesystem::path path(__FILE__);
    hannac::HLexer lexer{hannac::HFileParser{path.parent_path().string() + "/data/" + "real_numbers_incorrect.hanna"}};

    bool except{false};
    try
    {
        lexer.get_token();
    }
    catch (const std::exception &e)
    {
        except = true;
    }
    ASSERT_EQ(true, except);
}

TEST(HLexer, Method)
{
    std::filesystem::path path(__FILE__);
    hannac::HLexer lexer{hannac::HFileParser{path.parent_path().string() + "/data/" + "method.hanna"}};

    std::vector<hannac::HTokenRes> tokens;

    hannac::HTokenRes current{};
    while ((current = move_parser_helper(lexer)).first != hannac::HTokenType::END)
    {
        tokens.push_back(current);
    }
    EXPECT_EQ(tokens.size(), 2);
    hannac::HTokenRes one{hannac::HTokenType::Method, hannac::HToken{"method"}};
    hannac::HTokenRes two{hannac::HTokenType::Identifier, hannac::HToken{"abc"}};
    EXPECT_EQ(one.first, tokens[0].first);
    EXPECT_EQ(std::get<std::string>(one.second), std::get<std::string>(tokens[0].second));
    EXPECT_EQ(two.first, tokens[1].first);
    EXPECT_EQ(std::get<std::string>(two.second), std::get<std::string>(tokens[1].second));
}