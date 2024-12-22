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

TEST(HLexer, Comments)
{
    std::filesystem::path path(__FILE__);
    hannac::HLexer lexer{hannac::HFileParser{path.parent_path().string() + "/data/" + "comment.hanna"}};

    std::vector<hannac::HToken> tokens;

    std::pair<hannac::HTokenType, hannac::HToken> current{};
    while ((current = lexer.get_token()).first != hannac::HTokenType::END)
    {
        tokens.push_back(current.second);
    }

    EXPECT_EQ(tokens.size(), 0);
}

TEST(HLexer, Numbers)
{
    std::filesystem::path path(__FILE__);
    hannac::HLexer lexer{hannac::HFileParser{path.parent_path().string() + "/data/" + "numbers.hanna"}};

    std::vector<hannac::HToken> tokens;

    std::pair<hannac::HTokenType, hannac::HToken> current{};
    while ((current = lexer.get_token()).first != hannac::HTokenType::END)
    {
        tokens.push_back(current.second);
    }

    EXPECT_EQ(tokens.size(), 3);
    std::vector<hannac::NumArray> expected{{123}, {456, 999}, {789}};
    int i = 0;
    for (auto const &token : tokens)
    {
        EXPECT_EQ(expected[i], std::get<hannac::NumArray>(token));
        i++;
    }
}

TEST(HLexer, Method)
{
    std::filesystem::path path(__FILE__);
    hannac::HLexer lexer{hannac::HFileParser{path.parent_path().string() + "/data/" + "method.hanna"}};

    std::vector<std::pair<hannac::HTokenType, hannac::HToken>> tokens;

    std::pair<hannac::HTokenType, hannac::HToken> current{};
    while ((current = lexer.get_token()).first != hannac::HTokenType::END)
    {
        tokens.push_back(current);
    }
    EXPECT_EQ(tokens.size(), 2);
    std::pair<hannac::HTokenType, hannac::HToken> one{hannac::HTokenType::Method, hannac::HToken{"method"}};
    std::pair<hannac::HTokenType, hannac::HToken> two{hannac::HTokenType::Identifier, hannac::HToken{"abc"}};
    EXPECT_EQ(one.first, tokens[0].first);
    EXPECT_EQ(std::get<std::string>(one.second), std::get<std::string>(tokens[0].second));
    EXPECT_EQ(two.first, tokens[1].first);
    EXPECT_EQ(std::get<std::string>(two.second), std::get<std::string>(tokens[1].second));
}