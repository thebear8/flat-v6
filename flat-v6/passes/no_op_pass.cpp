#include "no_op_pass.hpp"

void NoOpPass::visit(IntegerExpression* node)
{
}

void NoOpPass::visit(BoolExpression* node)
{
}

void NoOpPass::visit(CharExpression* node)
{
}

void NoOpPass::visit(StringExpression* node)
{
}

void NoOpPass::visit(IdentifierExpression* node)
{
}

void NoOpPass::visit(StructExpression* node)
{
	for (auto& [name, value] : node->fields)
		dispatch(value);
}

void NoOpPass::visit(UnaryExpression* node)
{
	dispatch(node->expression);
}

void NoOpPass::visit(BinaryExpression* node)
{
	dispatch(node->left);
	dispatch(node->right);
}

void NoOpPass::visit(CallExpression* node)
{
	dispatch(node->expression);
	for (auto& value : node->args)
		dispatch(value);
}

void NoOpPass::visit(BoundCallExpression* node)
{
	for (auto& value : node->args)
		dispatch(value);
}

void NoOpPass::visit(IndexExpression* node)
{
	dispatch(node->expression);
	for (auto& value : node->args)
		dispatch(value);
}

void NoOpPass::visit(BoundIndexExpression* node)
{
	dispatch(node->expression);
	dispatch(node->index);
}

void NoOpPass::visit(FieldExpression* node)
{
	dispatch(node->expression);
}

void NoOpPass::visit(BlockStatement* node)
{
	for (auto& statement : node->statements)
		dispatch(statement);
}

void NoOpPass::visit(ExpressionStatement* node)
{
	dispatch(node->expression);
}

void NoOpPass::visit(VariableStatement* node)
{
	for (auto& [name, value] : node->items)
		dispatch(value);
}

void NoOpPass::visit(ReturnStatement* node)
{
	dispatch(node->expression);
}

void NoOpPass::visit(WhileStatement* node)
{
	dispatch(node->condition);
	dispatch(node->body);
}

void NoOpPass::visit(IfStatement* node)
{
	dispatch(node->condition);
	dispatch(node->ifBody);
	if (node->elseBody)
		dispatch(node->elseBody);
}

void NoOpPass::visit(StructDeclaration* node)
{
}

void NoOpPass::visit(FunctionDeclaration* node)
{
	dispatch(node->body);
}

void NoOpPass::visit(ExternFunctionDeclaration* node)
{
}

void NoOpPass::visit(Module* node)
{
	for (auto& declaration : node->declarations)
		dispatch(declaration);
}