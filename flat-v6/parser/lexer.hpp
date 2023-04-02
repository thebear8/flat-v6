#pragma once
#include <ostream>
#include <string>
#include <string_view>

#include "../data/token.hpp"
#include "../util/error_logger.hpp"

class Lexer
{
private:
    static constexpr std::pair<Token, std::string_view> tokens[] = {
        { Token::NewLine, "\n" },     { Token::NewLine, "\r\n" },

        { Token::Dot, "." },          { Token::Comma, "," },
        { Token::Colon, ":" },

        { Token::ParenOpen, "(" },    { Token::ParenClose, ")" },
        { Token::BracketOpen, "[" },  { Token::BracketClose, "]" },
        { Token::BraceOpen, "{" },    { Token::BraceClose, "}" },

        { Token::Plus, "+" },         { Token::Minus, "-" },
        { Token::LogicalNot, "!" },   { Token::BitwiseNot, "~" },
        { Token::Multiply, "*" },     { Token::Divide, "/" },
        { Token::Modulo, "%" },       { Token::ShiftLeft, "<<" },
        { Token::ShiftRight, ">>" },  { Token::BitwiseAnd, "&" },
        { Token::BitwiseOr, "|" },    { Token::BitwiseXor, "^" },
        { Token::Equal, "==" },       { Token::NotEqual, "!=" },
        { Token::LessThan, "<" },     { Token::GreaterThan, ">" },
        { Token::LessOrEqual, "<=" }, { Token::GreaterOrEqual, ">=" },
        { Token::LogicalAnd, "&&" },  { Token::LogicalOr, "||" },
        { Token::Assign, "=" },
    };

    static constexpr std::pair<Token, std::string_view> keywords[] = {
        { Token::True, "true" },     { Token::False, "false" },
        { Token::Let, "let" },       { Token::Return, "return" },
        { Token::While, "while" },   { Token::If, "if" },
        { Token::Else, "else" },     { Token::Struct, "struct" },
        { Token::Function, "fn" },   { Token::Constraint, "constraint" },
        { Token::Import, "import" }, { Token::Module, "module" },
        { Token::Function, "fn" },   { Token::Extern, "extern" },
    };

protected:
    ErrorLogger& logger;
    std::size_t position;
    std::string_view input;
    std::string_view value;
    std::string_view intValue;
    std::string_view intSuffixValue;
    std::size_t id;

public:
    Lexer(ErrorLogger& logger, std::string_view input, std::size_t id)
        : logger(logger), input(input), id(id), position(0), value("")
    {
    }

private:
    Token advance();
    std::string_view advanceChar();

public:
    std::size_t trim();
    bool match(Token expected);
    bool expect(Token expected);

public:
    std::string getTokenValue() { return std::string(value); };
    std::string getIntValue() { return std::string(intValue); };
    std::string getIntSuffixValue() { return std::string(intSuffixValue); };

private:
    bool isDigit(char c) { return (c >= '0' && c <= '9'); }
    bool isBinaryDigit(char c) { return (c >= '0' && c <= '1'); }
    bool isHexDigit(char c)
    {
        return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f')
            || (c >= 'A' && c <= 'F');
    }
    bool isLetter(char c)
    {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
    }
    bool isWhitespace(char c)
    {
        return (c == ' ' || c == '\t' || c == '\r' || c == '\n');
    }
    bool isIdentifier(char c)
    {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')
            || (c >= '0' && c <= '9') || (c == '_');
    }
};