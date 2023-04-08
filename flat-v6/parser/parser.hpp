#pragma once
#include "../ast/ast.hpp"
#include "../util/graph_context.hpp"
#include "lexer.hpp"

class Parser : protected Lexer
{
private:
    ErrorLogger& logger;
    GraphContext& ctx;
    std::size_t id;

public:
    Parser(
        ErrorLogger& logger,
        GraphContext& ctx,
        std::string_view input,
        std::size_t id
    )
        : Lexer(logger, input, id), logger(logger), ctx(ctx), id(id)
    {
    }

public:
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

private:
    std::vector<std::string> typeParamList();
    std::vector<ASTType*> typeArgList();
    std::vector<ASTRequirement*> requirementList();
};