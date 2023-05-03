#pragma once
#include <initializer_list>
#include <string_view>

#include "enum.hpp"

#define TOKEN_LIST(ENTRY) \
    ENTRY(Invalid)        \
    ENTRY(Error)          \
    ENTRY(Eof)            \
    ENTRY(Space)          \
    ENTRY(Tab)            \
    ENTRY(NewLine)        \
    ENTRY(Comment)        \
    ENTRY(Dot)            \
    ENTRY(Comma)          \
    ENTRY(Colon)          \
    ENTRY(ParenOpen)      \
    ENTRY(ParenClose)     \
    ENTRY(BracketOpen)    \
    ENTRY(BracketClose)   \
    ENTRY(BraceOpen)      \
    ENTRY(BraceClose)     \
    ENTRY(Plus)           \
    ENTRY(Minus)          \
    ENTRY(LogicalNot)     \
    ENTRY(BitwiseNot)     \
    ENTRY(Multiply)       \
    ENTRY(Divide)         \
    ENTRY(Modulo)         \
    ENTRY(ShiftLeft)      \
    ENTRY(ShiftRight)     \
    ENTRY(BitwiseAnd)     \
    ENTRY(BitwiseOr)      \
    ENTRY(BitwiseXor)     \
    ENTRY(Equal)          \
    ENTRY(NotEqual)       \
    ENTRY(LessThan)       \
    ENTRY(GreaterThan)    \
    ENTRY(LessOrEqual)    \
    ENTRY(GreaterOrEqual) \
    ENTRY(LogicalAnd)     \
    ENTRY(LogicalOr)      \
    ENTRY(Assign)         \
    ENTRY(True)           \
    ENTRY(False)          \
    ENTRY(Let)            \
    ENTRY(Return)         \
    ENTRY(While)          \
    ENTRY(If)             \
    ENTRY(Else)           \
    ENTRY(Struct)         \
    ENTRY(Function)       \
    ENTRY(Constraint)     \
    ENTRY(Extern)         \
    ENTRY(Import)         \
    ENTRY(Module)         \
    ENTRY(Integer)        \
    ENTRY(Identifier)     \
    ENTRY(CharLiteral)    \
    ENTRY(StringLiteral)

DEFINE_ENUM(Token, TOKEN_LIST)

namespace TokenDefinitions
{
constexpr std::pair<std::string_view, Token> keywords[] = {
    { "true", Token::True },     { "false", Token::False },
    { "let", Token::Let },       { "return", Token::Return },
    { "while", Token::While },   { "if", Token::If },
    { "else", Token::Else },     { "struct", Token::Struct },
    { "fn", Token::Function },   { "constraint", Token::Constraint },
    { "import", Token::Import }, { "module", Token::Module },
    { "fn", Token::Function },   { "extern", Token::Extern },
};

constexpr std::pair<std::string_view, Token> tokens[] = {
    { ".", Token::Dot },          { ",", Token::Comma },
    { ":", Token::Colon },

    { "(", Token::ParenOpen },    { ")", Token::ParenClose },
    { "[", Token::BracketOpen },  { "]", Token::BracketClose },
    { "{", Token::BraceOpen },    { "}", Token::BraceClose },

    { "+", Token::Plus },         { "-", Token::Minus },
    { "!", Token::LogicalNot },   { "~", Token::BitwiseNot },
    { "*", Token::Multiply },     { "/", Token::Divide },
    { "%", Token::Modulo },       { "<<", Token::ShiftLeft },
    { ">>", Token::ShiftRight },  { "&", Token::BitwiseAnd },
    { "|", Token::BitwiseOr },    { "^", Token::BitwiseXor },
    { "==", Token::Equal },       { "!=", Token::NotEqual },
    { "<", Token::LessThan },     { ">", Token::GreaterThan },
    { "<=", Token::LessOrEqual }, { ">=", Token::GreaterOrEqual },
    { "&&", Token::LogicalAnd },  { "||", Token::LogicalOr },
    { "=", Token::Assign },
};
};