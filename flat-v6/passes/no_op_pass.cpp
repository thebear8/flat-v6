#include "no_op_pass.hpp"

ASTNode* NoOpPass::visit(ASTIntegerExpression* node) {
    return node;
}

ASTNode* NoOpPass::visit(ASTBoolExpression* node) {
    return node;
}

ASTNode* NoOpPass::visit(ASTCharExpression* node) {
    return node;
}

ASTNode* NoOpPass::visit(ASTStringExpression* node) {
    return node;
}

ASTNode* NoOpPass::visit(ASTIdentifierExpression* node) {
    return node;
}

ASTNode* NoOpPass::visit(ASTStructExpression* node) {
    for (auto& [name, value] : node->fields)
        dispatch(value);
    return node;
}

ASTNode* NoOpPass::visit(ASTUnaryExpression* node) {
    dispatch(node->expression);
    return node;
}

ASTNode* NoOpPass::visit(ASTBinaryExpression* node) {
    dispatch(node->left);
    dispatch(node->right);
    return node;
}

ASTNode* NoOpPass::visit(ASTCallExpression* node) {
    dispatch(node->expression);
    for (auto& value : node->args)
        dispatch(value);
    return node;
}

ASTNode* NoOpPass::visit(ASTBoundCallExpression* node) {
    for (auto& value : node->args)
        dispatch(value);
    return node;
}

ASTNode* NoOpPass::visit(ASTIndexExpression* node) {
    dispatch(node->expression);
    for (auto& value : node->args)
        dispatch(value);
    return node;
}

ASTNode* NoOpPass::visit(ASTBoundIndexExpression* node) {
    dispatch(node->expression);
    dispatch(node->index);
    return node;
}

ASTNode* NoOpPass::visit(ASTFieldExpression* node) {
    dispatch(node->expression);
    return node;
}

ASTNode* NoOpPass::visit(ASTBlockStatement* node) {
    for (auto& statement : node->statements)
        dispatch(statement);
    return node;
}

ASTNode* NoOpPass::visit(ASTExpressionStatement* node) {
    dispatch(node->expression);
    return node;
}

ASTNode* NoOpPass::visit(ASTVariableStatement* node) {
    for (auto& [name, value] : node->items)
        dispatch(value);
    return node;
}

ASTNode* NoOpPass::visit(ASTReturnStatement* node) {
    dispatch(node->expression);
    return node;
}

ASTNode* NoOpPass::visit(ASTWhileStatement* node) {
    dispatch(node->condition);
    dispatch(node->body);
    return node;
}

ASTNode* NoOpPass::visit(ASTIfStatement* node) {
    dispatch(node->condition);
    dispatch(node->ifBody);
    if (node->elseBody)
        dispatch(node->elseBody);
    return node;
}

ASTNode* NoOpPass::visit(ASTStructDeclaration* node) {
    return node;
}

ASTNode* NoOpPass::visit(ASTFunctionDeclaration* node) {
    dispatch(node->body);
    return node;
}

ASTNode* NoOpPass::visit(ASTExternFunctionDeclaration* node) {
    return node;
}

ASTNode* NoOpPass::visit(ASTSourceFile* node) {
    for (auto& decl : node->declarations)
        dispatch(decl);

    return node;
}