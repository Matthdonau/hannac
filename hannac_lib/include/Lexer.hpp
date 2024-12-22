#ifndef LEXER_HPP
#define LEXER_HPP

// stdlib includes
#include <cctype>
#include <cstdint>
#include <utility>
#include <variant>

// hannac includes
#include "FileParser.hpp"

namespace hannac
{
using NumArray = std::vector<std::int64_t>;
using HToken = std::variant<std::string, NumArray, char>;

struct TokenError : public std::exception
{
  public:
    TokenError(std::string const &message) : mMessage{message}
    {
    }

    const char *what() const throw()
    {
        return mMessage.c_str();
    }

  private:
    std::string mMessage;
};

enum class HTokenType : std::uint8_t
{
    // End of file.
    END = 0,
    Character = 1,

    // Functional.
    Method = 2,

    // Primary.
    Identifier = 3,
    NumArray = 4,
};

struct HLexer final
{
  public:
    HLexer(HFileParser &&parser) : mParser(std::move(parser))
    {
    }

    std::pair<HTokenType, HToken> get_token()
    {
        HToken token;

        // Skip ahead all whitespaces.
        char current;
        while (std::isspace(current = mParser.read()))
            ;

        // Handle alphanumeric strings.
        if (current == EOF)
        {
            return {HTokenType::END, std::string{current}};
        }
        else if (std::isalpha(current))
        {
            std::string result{current};
            while (std::isalnum(current = mParser.read()))
                result += current;

            if (result == "method")
                return {HTokenType::Method, result};
            else
                return {HTokenType::Identifier, result};
        }
        else if (std::isdigit(current))
        {
            NumArray result;
            while (std::isdigit(current) || current == ' ')
            {
                std::string number{};

                if (std::isspace(current))
                    current = mParser.read();

                while (std::isdigit(current))
                {
                    number += current;
                    current = mParser.read();
                }
                result.push_back(static_cast<std::int64_t>(std::stoll(number)));
            }

            return {HTokenType::NumArray, result};
        }
        else if (current == '#') // Skip comments
        {
            do
                current = mParser.read();
            while (current != EOF && current != '\n' && current != '\r');

            // Skipped comment call this function once again.
            return get_token();
        }
        else
        {
            return {HTokenType::Character, current};
        }
    }

  private:
    HFileParser mParser;
};
} // namespace hannac
#endif // LEXER_HPP