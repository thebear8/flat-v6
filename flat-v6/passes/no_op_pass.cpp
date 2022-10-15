#include "no_op_pass.hpp"

AstNode* NoOpPass::visit(IntegerExpression* node)
{
	return node;
}

AstNode* NoOpPass::visit(BoolExpression* node)
{
	return node;
}

AstNode* NoOpPass::visit(CharExpression* node)
{
	return node;
}

AstNode* NoOpPass::visit(StringExpression* node)
{
	return node;
}

AstNode* NoOpPass::visit(IdentifierExpression* node)
{
	return node;
}

AstNode* NoOpPass::visit(StructExpression* node)
{
	for (auto& [name, value] : node->fields)
		dispatch(value);
	return node;
}

AstNode* NoOpPass::visit(UnaryExpression* node)
{
	dispatch(node->expression);
	return node;
}

AstNode* NoOpPass::visit(BinaryExpression* node)
{
	dispatch(node->left);
	dispatch(node->right);
	return node;
}

AstNode* NoOpPass::visit(CallExpression* node)
{
	dispatch(node->expression);
	for (auto& value : node->args)
		dispatch(value);
	return node;
}

AstNode* NoOpPass::visit(BoundCallExpression* node)
{
	for (auto& value : node->args)
		dispatch(value);
	return node;
}

AstNode* NoOpPass::visit(IndexExpression* node)
{
	dispatch(node->expression);
	for (auto& value : node->args)
		dispatch(value);
	return node;
}

AstNode* NoOpPass::visit(BoundIndexExpression* node)
{
	dispatch(node->expression);
	dispatch(node->index);
	return node;
}

AstNode* NoOpPass::visit(FieldExpression* node)
{
	dispatch(node->expression);
	return node;
}

AstNode* NoOpPass::visit(BlockStatement* node)
{
	for (auto& statement : node->statements)
		dispatch(statement);
	return node;
}

AstNode* NoOpPass::visit(ExpressionStatement* node)
{
	dispatch(node->expression);
	return node;
}

AstNode* NoOpPass::visit(VariableStatement* node)
{
	for (auto& [name, value] : node->items)
		dispatch(value);
	return node;
}

AstNode* NoOpPass::visit(ReturnStatement* node)
{
	dispatch(node->expression);
	return node;
}

AstNode* NoOpPass::visit(WhileStatement* node)
{
	dispatch(node->condition);
	dispatch(node->body);
	return node;
}

AstNode* NoOpPass::visit(IfStatement* node)
{
	dispatch(node->condition);
	dispatch(node->ifBody);
	if (node->elseBody)
		dispatch(node->elseBody);
	return node;
}

AstNode* NoOpPass::visit(StructDeclaration* node)
{
	return node;
}

AstNode* NoOpPass::visit(FunctionDeclaration* node)
{
	dispatch(node->body);
	return node;
}

AstNode* NoOpPass::visit(ExternFunctionDeclaration* node)
{
	return node;
}

AstNode* NoOpPass::visit(Module* node)
{
	for (auto& declaration : node->declarations)
		dispatch(declaration);
	return node;
}