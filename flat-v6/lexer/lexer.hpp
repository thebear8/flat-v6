#include <cstddef>
#include <string>
#include <string_view>
#include <unordered_map>

#include "../data/token.hpp"

class ErrorLogger;

struct TokenInfo
{
public:
    Token token = Token::Invalid;
    std::size_t begin = 0, end = 0;
    std::string_view value = {};

public:
    constexpr inline TokenInfo() = default;
    constexpr inline TokenInfo(TokenInfo&&) = default;
    constexpr inline TokenInfo(TokenInfo const&) = default;
    constexpr inline TokenInfo& operator=(TokenInfo&&) = default;
    constexpr inline TokenInfo& operator=(TokenInfo const&) = default;

    constexpr inline TokenInfo(size_t position) : begin(position), end(position)
    {
    }

    constexpr inline TokenInfo(Token token, size_t position)
        : token(token), begin(position), end(position)
    {
    }

    constexpr inline TokenInfo(
        Token token, std::size_t begin, std::size_t end, std::string_view value
    )
        : token(token), begin(begin), end(end), value(value)
    {
    }

    constexpr auto valid() const { return token != Token::Invalid; }
    constexpr inline operator bool() const { return valid(); }
};

class Lexer
{
private:
    size_t m_position = 0;
    std::string_view m_input = {};

    std::unordered_map<std::string_view, Token> const& m_keywords;
    std::unordered_map<size_t, std::unordered_map<std::string_view, Token>>
        m_tokens;

public:
    Lexer(
        std::unordered_map<std::string_view, Token> const& tokens,
        std::unordered_map<std::string_view, Token> const& keywords
    );

    auto getPosition() { return m_position; }
    auto setInput(std::string_view input) { return (m_input = input); }
    auto setPosition(size_t position) { return (m_position = position); }

    size_t trim();
    TokenInfo match(Token token);
    TokenInfo lookahead(Token token);

    TokenInfo advance();
    TokenInfo advanceWhitespace();

    bool inBounds() { return (m_position < m_input.size()); }

private:
    TokenInfo advanceIdentifier();
    TokenInfo advanceInteger();
    TokenInfo advanceCharLiteral();
    TokenInfo advanceStringLiteral();
    TokenInfo advanceFixedToken();

    std::string_view eatChar();

    inline bool peek(char c)
    {
        return inBounds() && (m_input[m_position] == c);
    }

    inline bool peek(char start, char end)
    {
        return inBounds()
            && (m_input[m_position] >= start && m_input[m_position] <= end);
    }

    inline bool peek(std::string_view s)
    {
        return inBounds() && (m_input.substr(m_position, s.length()) == s);
    }

    inline bool eat(char c) { return peek(c) && (++m_position, true); }

    inline bool eat(char start, char end)
    {
        return peek(start, end) && (++m_position, true);
    }

    inline bool eat(std::string_view s)
    {
        return peek(s) && (m_position += s.length(), true);
    }
};