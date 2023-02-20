#pragma once
#include "../ast/ast.hpp"
#include "../util/graph_context.hpp"
#include "lexer.hpp"

class Parser : protected Lexer
{
private:
    ErrorLogger& logger;
    GraphContext& ctx;
    size_t id;

public:
    Parser(
        ErrorLogger& logger,
        GraphContext& ctx,
        std::string_view input,
        size_t id
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

    ASTStatement* blockStatement(size_t begin);
    ASTStatement* variableStatement(size_t begin);
    ASTStatement* returnStatement(size_t begin);
    ASTStatement* whileStatement(size_t begin);
    ASTStatement* ifStatement(size_t begin);
    ASTStatement* statement();

    ASTConstraintDeclaration* constraintDeclaration(size_t begin);
    ASTStructDeclaration* structDeclaration(size_t begin);
    ASTFunctionDeclaration* functionDeclaration(size_t begin);
    ASTFunctionDeclaration* externFunctionDeclaration(size_t begin);

    ASTSourceFile* sourceFile();

    ASTType* typeName();

private:
    std::vector<std::string> typeParamList();
    std::vector<ASTType*> typeArgList();
    std::vector<std::pair<std::string, std::vector<ASTType*>>> constraintList();
};