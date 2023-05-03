#pragma once
#include "../ast/ast.hpp"
#include "../lexer/lexer.hpp"
#include "../util/graph_context.hpp"

class Parser
{
private:
    ErrorLogger& m_logger;
    Lexer& m_lexer;
    GraphContext& m_ctx;

    std::size_t m_id = 0;
    std::string_view m_tokenValue = {};

public:
    Parser(ErrorLogger& logger, Lexer& lexer, GraphContext& ctx)
        : m_logger(logger), m_lexer(lexer), m_ctx(ctx), m_id(m_id)
    {
    }

    ASTExpression* l0();
    ASTExpression* l1();
    ASTExpression* l2();
    ASTExpression* l3();
    ASTExpression* l4();
    ASTExpression* l5();
    ASTExpression* l6();
    ASTExpression* l7();
    ASTExpression* l8();
    ASTExpression* l9();
    ASTExpression* l10();
    ASTExpression* expression();

    ASTStatement* expressionStatement(std::size_t begin);
    ASTStatement* blockStatement(std::size_t begin);
    ASTStatement* variableStatement(std::size_t begin);
    ASTStatement* returnStatement(std::size_t begin);
    ASTStatement* whileStatement(std::size_t begin);
    ASTStatement* ifStatement(std::size_t begin);
    ASTStatement* statement();

    ASTConstraintCondition* constraintCondition(std::size_t begin);
    ASTConstraintDeclaration* constraintDeclaration(std::size_t begin);
    ASTStructDeclaration* structDeclaration(std::size_t begin);
    ASTFunctionDeclaration* functionDeclaration(std::size_t begin);
    ASTFunctionDeclaration* externFunctionDeclaration(std::size_t begin);

    ASTSourceFile* sourceFile();

    ASTType* typeName();

    std::vector<std::string> typeParamList();
    std::vector<ASTType*> typeArgList();
    std::vector<ASTRequirement*> requirementList();

private:
    std::size_t trim();
    TokenInfo match(Token token);
    TokenInfo expect(Token token);
    TokenInfo lookahead(Token token);

    std::size_t getPosition() { return m_lexer.getPosition(); }
    std::string_view getTokenValue() { return m_tokenValue; }
};