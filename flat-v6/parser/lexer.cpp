#include "lexer.hpp"

#include "../util/error_logger.hpp"

Token Lexer::advance()
{
    if (position >= input.length())
    {
        value = "";
        return Token::Eof;
    }

    std::size_t length = 0;
    Token type = (Token)(-1);
    for (auto& token : tokens)
    {
        if (length < token.second.length()
            && input.substr(position, token.second.length()) == token.second)
        {
            length = token.second.length();
            type = token.first;
        }
    }

    if (type != (Token)(-1))
    {
        value = input.substr(position, length);
        position += length;
        return type;
    }

    if (isDigit(input[position]))
    {
        std::size_t start = position;
        if (input[position] == '0' && (position + 1) < input.length()
            && input[position + 1] == 'b')
        {
            position += 2;
            while (position < input.length() && isBinaryDigit(input[position]))
                position++;
        }
        else if (input[position] == '0' && (position + 1) < input.length() && input[position + 1] == 'x')
        {
            position += 2;
            while (position < input.length() && isHexDigit(input[position]))
                position++;
        }
        else
        {
            while (position < input.length() && isDigit(input[position]))
                position++;
        }

        intValue = input.substr(start, position - start);

        std::size_t suffixStart = position;
        if (position < input.length()
            && (input[position] == 'i' || input[position] == 'u'))
        {
            position++;
            while (position < input.length() && isDigit(input[position]))
                position++;
        }

        intSuffixValue = input.substr(suffixStart, position - suffixStart);
        value = input.substr(start, position - start);
        return Token::Integer;
    }

    if (isIdentifier(input[position]))
    {
        std::size_t start = position;
        while (position < input.length() && isIdentifier(input[position]))
            position++;
        value = input.substr(start, position - start);

        for (auto& keyword : keywords)
        {
            if (input.substr(start, position - start) == keyword.second)
            {
                return keyword.first;
            }
        }

        return Token::Identifier;
    }

    if (input[position] == '\'')
    {
        position++;
        std::size_t start = position;
        if (input[position] == '\'')
            logger.error(SourceRef(id, position), "Empty char literal");

        advanceChar();

        if (!(position < input.length() && input[position] == '\''))
            logger.error(SourceRef(id, position), "Unterminated char literal");

        value = input.substr(start, (position - start));
        position++;
        return Token::CharLiteral;
    }

    if (input[position] == '\"')
    {
        position++;
        std::size_t start = position;

        while (position < input.length() && input[position] != '\"')
            advanceChar();

        if (!(position < input.length() && input[position] == '\"'))
            logger.error(
                SourceRef(id, position), "Unterminated string literal"
            );

        value = input.substr(start, (position - start));
        position++;
        return Token::StringLiteral;
    }

    return Token::Error;
}

std::string_view Lexer::advanceChar()
{
    std::size_t start = position;
    if (position < input.length() && input[position] == '\\')
    {
        position++;
        if (position < input.length()
            && isDigit(input[position]))  // octal char literal
        {
            std::size_t numStart = position;
            while (position < input.length() && isDigit(input[position]))
                position++;

            if ((position - numStart) > 3)
                logger.error(
                    SourceRef(id, numStart),
                    "Too many digits for octal char literal"
                );

            return input.substr(start, (position - start));
        }
        else if (position < input.length() && input[position] == 'x')  // hex
                                                                       // char
                                                                       // literal
        {
            position++;
            std::size_t numStart = position;
            while (position < input.length() && isDigit(input[position]))
                position++;

            if ((position - numStart) == 0)
                logger.error(
                    SourceRef(id, numStart),
                    "Hex char literal cannot have zero digits"
                );

            return input.substr(start, (position - start));
        }
        else if (position < input.length() && input[position] == 'u')  // 0xhhhh
                                                                       // unicode
                                                                       // code
                                                                       // point
        {
            position++;
            std::size_t numStart = position;
            while (position < input.length() && isDigit(input[position]))
                position++;

            if ((position - numStart) != 4)
                logger.error(
                    SourceRef(id, numStart),
                    "2 byte Unicode code point (\\u) must have 4 digits"
                );

            return input.substr(start, (position - start));
        }
        else if (position < input.length() && input[position] == 'U')  // 0xhhhhhhhh
                                                                       // unicode
                                                                       // code
                                                                       // point
        {
            position++;
            std::size_t numStart = position;
            while (position < input.length() && isDigit(input[position]))
                position++;

            if ((position - numStart) != 8)
                logger.error(
                    SourceRef(id, numStart),
                    "4 byte Unicode code point (\\U) must have 8 digits"
                );

            return input.substr(start, (position - start));
        }
        else if (position < input.length())
        {
            if (std::string("abefnrtv\\\'\"\?").find(input[position])
                == std::string::npos)
                logger.error(
                    SourceRef(id, position), "Invalid escape sequence"
                );

            position++;
            return input.substr(start, (position - start));
        }
        else
        {
            logger.error(
                SourceRef(id, position), "Unexpected EOF in escape sequence"
            );
            return "";
        }
    }
    else if (position < input.length())
    {
        position++;
        return input.substr(start, (position - start));
    }
    else
    {
        logger.error(SourceRef(id, position), "Unexpected EOF");
        return "";
    }
}

std::size_t Lexer::trim()
{
    while (true)
    {
        if (position < input.length() && isWhitespace(input[position]))
        {
            position++;
        }
        else if ((position < input.length() && input[position] == '/')
            && ((position + 1) < input.length() && input[position + 1] == '/'))
        {
            while (position < input.length() && input[position] != '\n')
                position++;
            position++;
        }
        else
        {
            return position;
        }
    }
}

bool Lexer::peek(Token expected)
{
    std::size_t before = position;
    Token token = advance();
    position = before;

    if (token == expected)
        return true;

    trim();
    token = advance();
    position = before;

    if (token == expected)
        return true;
    else if (token == Token::Error)
        logger.error(SourceRef(id, position), "Invalid Token");

    return false;
}

bool Lexer::match(Token expected)
{
    std::size_t before = position;
    Token token = advance();
    if (token == expected)
        return true;

    position = before;
    trim();
    token = advance();
    if (token == Token::Error)
        logger.error(SourceRef(id, position), "Invalid Token");
    else if (token == expected)
        return true;

    position = before;
    return false;
}

bool Lexer::expect(Token expected)
{
    std::size_t before = position;
    Token token = advance();
    if (token == expected)
        return true;

    position = before;
    trim();
    token = advance();
    if (token == Token::Error)
        logger.error(SourceRef(id, position), "Invalid Token");
    else if (token == expected)
        return true;

    position = before;
    logger.error(
        SourceRef(id, position),
        "Unexpected Token " + std::string(value) + ", expected "
            + TokenNames[(std::size_t)expected]
    );
    return false;
}