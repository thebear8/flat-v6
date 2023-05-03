#include "lexer.hpp"

#include "../data/source_ref.hpp"
#include "../util/error_logger.hpp"

Lexer::Lexer(
    std::unordered_map<std::string_view, Token> const& keywords,
    std::unordered_map<std::string_view, Token> const& tokens
)
    : m_keywords(keywords)
{
    for (auto const& [str, token] : tokens)
        m_tokens[str.length()].try_emplace(str, token);
}

size_t Lexer::trim()
{
    while (advanceWhitespace()) {}
    return m_position;
}

TokenInfo Lexer::match(Token token)
{
    size_t position = m_position;

    TokenInfo next = {};
    while (next = advanceWhitespace())
    {
        if (next.token == token)
        {
            return next;
        }
    }

    next = advance();
    m_position = (next.token == token) ? (m_position) : (position);
    return (next.token == token) ? (next) : (TokenInfo(position));
}

TokenInfo Lexer::lookahead(Token token)
{
    size_t position = m_position;

    TokenInfo next = {};
    while (next = advanceWhitespace())
    {
        if (next.token == token)
        {
            m_position = position;
            return next;
        }
    }

    next = advance();
    m_position = position;
    return (next.token == token) ? (next) : (TokenInfo(position));
}

TokenInfo Lexer::advance()
{
    TokenInfo next = {};
    if (next = advanceIdentifier())
    {
        return next;
    }
    else if (next = advanceInteger())
    {
        return next;
    }
    else if (next = advanceCharLiteral())
    {
        return next;
    }
    else if (next = advanceStringLiteral())
    {
        return next;
    }

    return TokenInfo(Token::Error, m_position);
}

TokenInfo Lexer::advanceIdentifier()
{
    size_t begin = m_position;
    if (!peek('a', 'z') && !peek('A', 'Z') && !peek('_'))
        return TokenInfo(begin);

    while (eat('a', 'z') || eat('A', 'Z') || eat('0', '9') || eat('_')) {}

    auto v = m_input.substr(begin, (m_position - begin));
    auto token = m_keywords.contains(v) ? m_keywords.at(v) : Token::Identifier;
    return TokenInfo(token, begin, m_position, v);
}

TokenInfo Lexer::advanceInteger()
{
    size_t begin = m_position;
    if (!peek('0', '9'))
        return TokenInfo(begin);

    if (eat("0b"))
    {
        while (eat('0', '1')) {}
    }
    else if (eat("0x"))
    {
        while (eat('0', '9') || eat('a', 'f') || eat('A', 'F')) {}
    }
    else if (eat('0', '9'))
    {
        while (eat('0', '9')) {}
    }

    eat("i8") || eat("i16") || eat("i32") || eat("i64") || eat("u8")
        || eat("u16") || eat("u32") || eat("u64");

    return TokenInfo(
        Token::Integer,
        begin,
        m_position,
        m_input.substr(begin, (m_position - begin))
    );
}

TokenInfo Lexer::advanceCharLiteral()
{
    size_t begin = m_position;
    if (!eat("'"))
        return TokenInfo(begin);

    eatChar();
    eat("'");

    return TokenInfo(
        Token::CharLiteral,
        begin,
        m_position,
        m_input.substr(begin, (m_position - begin))
    );
}

TokenInfo Lexer::advanceStringLiteral()
{
    size_t begin = m_position;
    if (!eat('"'))
        return TokenInfo(begin);

    while (inBounds() && !eat('"'))
    {
        eatChar();
    }

    return TokenInfo(
        Token::StringLiteral,
        begin,
        m_position,
        m_input.substr(begin, (m_position - begin))
    );
}

TokenInfo Lexer::advanceFixedToken()
{
    size_t begin = m_position;
    for (auto const& [length, tokens] : m_tokens)
    {
        if ((m_position + length) >= m_input.length())
            break;

        auto value = m_input.substr(m_position, length);
        if (tokens.contains(value))
            return TokenInfo(tokens.at(value), begin, m_position, value);
    }

    return TokenInfo(begin);
}

TokenInfo Lexer::advanceWhitespace()
{
    size_t begin = m_position;
    if (eat(' '))
    {
        return TokenInfo(
            Token::Space,
            begin,
            m_position,
            m_input.substr(begin, (m_position - begin))
        );
    }
    else if (eat('\t'))
    {
        return TokenInfo(
            Token::Tab,
            begin,
            m_position,
            m_input.substr(begin, (m_position - begin))
        );
    }
    else if (eat('\n') || eat("\r\n"))
    {
        return TokenInfo(
            Token::Tab,
            begin,
            m_position,
            m_input.substr(begin, (m_position - begin))
        );
    }
    else if (eat("//"))
    {
        while (inBounds() && !eat('\n'))
        {
            ++m_position;
        }

        return TokenInfo(
            Token::Comment,
            begin,
            m_position,
            m_input.substr(begin, (m_position - begin))
        );
    }

    return TokenInfo(begin);
}

std::string_view Lexer::eatChar()
{
    size_t start = m_position;
    if (eat("\\o"))
    {
        while (eat('0', '7')) {}
    }
    else if (eat("\\d"))
    {
        while (eat('0', '9')) {}
    }
    else if (eat("\\x") || eat("\\u") || eat("\\U"))
    {
        while (eat('0', '9') || eat('a', 'f') || eat('A', 'F')) {}
    }
    else if (eat('\\'))
    {
        eat('a') || eat('b') || eat('e') || eat('f') || eat('n') || eat('r')
            || eat('t') || eat('v') || eat('\\') || eat('"') || eat("'")
            || eat("?");
    }
    else if (inBounds())
    {
        ++m_position;
    }

    return m_input.substr(start, (m_position - start));
}