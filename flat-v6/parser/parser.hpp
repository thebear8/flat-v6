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

    ASTStatement* expressionStatement();
    ASTStatement* blockStatement();
    ASTStatement* variableStatement();
    ASTStatement* returnStatement();
    ASTStatement* whileStatement();
    ASTStatement* ifStatement();
    ASTStatement* statement();

    ASTConstraintCondition* constraintCondition();
    ASTConstraintDeclaration* constraintDeclaration();
    ASTStructDeclaration* structDeclaration();
    ASTFunctionDeclaration* functionDeclaration();
    ASTFunctionDeclaration* externFunctionDeclaration();

    ASTSourceFile* sourceFile();

    ASTType* typeName();

private:
    std::vector<std::string> typeParamList();
    std::vector<ASTType*> typeArgList();
    std::vector<ASTRequirement*> requirementList();
};