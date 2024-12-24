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
// Forward declaration.
enum class HTokenType : std::uint8_t;

// Convenience names.
using NumArray = std::vector<std::int64_t>;
using HToken = std::variant<std::string, NumArray, char>;
using HTokenRes = std::pair<HTokenType, HToken>;

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
    explicit HLexer(HFileParser &&parser) : mParser(std::move(parser))
    {
    }

    HTokenRes get_token()
    {
        HToken token;

        // Skip ahead all whitespaces.
        while (std::isspace(mCurrent))
        {
            mCurrent = mParser.read();
        }

        // Handle alphanumeric strings.
        if (mCurrent == EOF)
        {
            return {HTokenType::END, std::string{mCurrent}};
        }
        else if (std::isalpha(mCurrent))
        {
            std::string result{mCurrent};
            while (std::isalnum(mCurrent = mParser.read()))
            {
                result += mCurrent;
            }

            if (result == "method")
                return {HTokenType::Method, result};
            else
                return {HTokenType::Identifier, result};
        }
        else if (std::isdigit(mCurrent))
        {
            NumArray result;
            while (std::isdigit(mCurrent) || mCurrent == ' ')
            {
                std::string number{};

                if (std::isspace(mCurrent))
                    mCurrent = mParser.read();

                while (std::isdigit(mCurrent))
                {
                    number += mCurrent;
                    mCurrent = mParser.read();
                }
                result.push_back(static_cast<std::int64_t>(std::stoll(number)));
            }

            return {HTokenType::NumArray, result};
        }
        else if (mCurrent == '#') // Skip comments
        {
            do
                mCurrent = mParser.read();
            while (mCurrent != EOF && mCurrent != '\n' && mCurrent != '\r');

            // Skipped comment call this function once again.
            return get_token();
        }
        else
        {
            char const ret = mCurrent;
            mCurrent = mParser.read();
            return {HTokenType::Character, ret};
        }
    }

  private:
    HFileParser mParser;
    char mCurrent = ' ';
};
} // namespace hannac
#endif // LEXER_HPP