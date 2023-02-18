#pragma once
#include "../data/ast.hpp"

class NoOpPass : public ASTVisitor<ASTNode*> {
    virtual ASTNode* visit(ASTIntegerExpression* node) override;
    virtual ASTNode* visit(ASTBoolExpression* node) override;
    virtual ASTNode* visit(ASTCharExpression* node) override;
    virtual ASTNode* visit(ASTStringExpression* node) override;
    virtual ASTNode* visit(ASTIdentifierExpression* node) override;
    virtual ASTNode* visit(ASTStructExpression* node) override;
    virtual ASTNode* visit(ASTUnaryExpression* node) override;
    virtual ASTNode* visit(ASTBinaryExpression* node) override;
    virtual ASTNode* visit(ASTCallExpression* node) override;
    virtual ASTNode* visit(ASTBoundCallExpression* node) override;
    virtual ASTNode* visit(ASTIndexExpression* node) override;
    virtual ASTNode* visit(ASTBoundIndexExpression* node) override;
    virtual ASTNode* visit(ASTFieldExpression* node) override;

    virtual ASTNode* visit(ASTBlockStatement* node) override;
    virtual ASTNode* visit(ASTExpressionStatement* node) override;
    virtual ASTNode* visit(ASTVariableStatement* node) override;
    virtual ASTNode* visit(ASTReturnStatement* node) override;
    virtual ASTNode* visit(ASTWhileStatement* node) override;
    virtual ASTNode* visit(ASTIfStatement* node) override;

    virtual ASTNode* visit(ASTStructDeclaration* node) override;
    virtual ASTNode* visit(ASTFunctionDeclaration* node) override;
    virtual ASTNode* visit(ASTExternFunctionDeclaration* node) override;
    virtual ASTNode* visit(ASTSourceFile* node) override;
};