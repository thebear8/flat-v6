#include "lowering_pass.hpp"

IRSourceFile* OperatorLoweringPass::process(IRSourceFile* program)
{
	return (IRSourceFile*)dispatch(program);
}

IRNode* OperatorLoweringPass::visit(IRIntegerExpression* node)
{
	return node;
}

IRNode* OperatorLoweringPass::visit(IRBoolExpression* node)
{
	return node;
}

IRNode* OperatorLoweringPass::visit(IRCharExpression* node)
{
	return node;
}

IRNode* OperatorLoweringPass::visit(IRStringExpression* node)
{
	return node;
}

IRNode* OperatorLoweringPass::visit(IRIdentifierExpression* node)
{
	return node;
}

IRNode* OperatorLoweringPass::visit(IRStructExpression* node)
{
	return node;
}

IRNode* OperatorLoweringPass::visit(IRUnaryExpression* node)
{
	node->expression = checked_cast<IRExpression>(dispatch(node->expression));

	auto value = node->expression->type;
	if (unaryOperators.at(node->operation).category == OperatorCategory::UnaryArithmetic && value->isIntegerType())
	{
		return node;
	}
	else if (unaryOperators.at(node->operation).category == OperatorCategory::UnaryBitwise && value->isIntegerType())
	{
		return node;
	}
	else if (unaryOperators.at(node->operation).category == OperatorCategory::UnaryLogic && value->isBoolType())
	{
		return node;
	}
	else
	{
		auto args = std::vector({ node->expression });
		auto identifier = modCtx.irCtx.make(IRIdentifierExpression(unaryOperators.at(node->operation).name));
		auto call = irCtx.make(IRCallExpression(identifier, args));
		call->type = node->type;
		return call;
	}
}

IRNode* OperatorLoweringPass::visit(IRBinaryExpression* node)
{
	node->left = checked_cast<IRExpression>(dispatch(node->left));
	node->right = checked_cast<IRExpression>(dispatch(node->right));

	auto left = node->left->type;
	auto right = node->right->type;

	if (binaryOperators.at(node->operation).category == OperatorCategory::BinaryArithmetic && (left->isIntegerType() && right->isIntegerType()))
	{
		return node;
	}
	else if (binaryOperators.at(node->operation).category == OperatorCategory::BinaryBitwise && (left->isIntegerType() && right->isIntegerType()) && (left->getBitSize() == right->getBitSize()))
	{
		return node;
	}
	else if (binaryOperators.at(node->operation).category == OperatorCategory::BinaryComparison && (left->isIntegerType() && right->isIntegerType()))
	{
		return node;
	}
	else if (binaryOperators.at(node->operation).category == OperatorCategory::BinaryLogic && (left->isBoolType() && right->isBoolType()))
	{
		return node;
	}
	else if (binaryOperators.at(node->operation).category == OperatorCategory::BinaryEquality && ((left == right) || (left->isIntegerType() && right->isIntegerType())))
	{
		return node;
	}
	else if (binaryOperators.at(node->operation).category == OperatorCategory::BinaryAssign && ((left == right) || (left->isIntegerType() && right->isIntegerType())))
	{
		return node;
	}
	else
	{
		if (node->operation == BinaryOperator::Assign)
		{
			auto args = std::vector({ node->left, node->right });
			auto identifier = irCtx.make(IRIdentifierExpression(binaryOperators.at(BinaryOperator::Assign).name));
			auto assignCall = irCtx.make(IRCallExpression(identifier, args));
			assignCall->type = node->type;

			auto assign = irCtx.make(IRBinaryExpression(BinaryOperator::Assign, node->left, assignCall));
			assign->type = node->type;
			return assign;
		}
		else
		{
			auto args = std::vector({ node->left, node->right });
			auto identifier = irCtx.make(IRIdentifierExpression(binaryOperators.at(node->operation).name));
			auto call = irCtx.make(IRCallExpression(identifier, args));
			call->type = node->type;
			return call;
		}
	}
}

IRNode* OperatorLoweringPass::visit(IRCallExpression* node)
{
	node->expression = checked_cast<IRExpression>(dispatch(node->expression));
	for (auto& arg : node->args)
		arg = checked_cast<IRExpression>(dispatch(arg));

	if (dynamic_cast<IRIdentifierExpression*>(node->expression))
		return node;

	auto args = node->args;
	args.insert(args.begin(), node->expression);
	auto identifier = irCtx.make(IRIdentifierExpression("__call__"));
	auto call = irCtx.make(IRCallExpression(identifier, args));
	call->type = node->type;
	return call;
}

IRNode* OperatorLoweringPass::visit(IRIndexExpression* node)
{
	node->expression = checked_cast<IRExpression>(dispatch(node->expression));
	for (auto& arg : node->args)
		arg = checked_cast<IRExpression>(dispatch(arg));

	auto value = node->expression->type;
	if ((value->isArrayType() || value->isStringType()) && node->args.size() == 1 && node->args.front()->type->isIntegerType())
		return node;

	auto args = node->args;
	args.insert(args.begin(), node->expression);
	auto identifier = irCtx.make(IRIdentifierExpression("__index"));
	auto call = irCtx.make(IRCallExpression(identifier, args));
	call->type = node->type;
	return call;
}

IRNode* OperatorLoweringPass::visit(IRFieldExpression* node)
{
	node->expression = checked_cast<IRExpression>(dispatch(node->expression));
	return node;
}

IRNode* OperatorLoweringPass::visit(IRBlockStatement* node)
{
	for (auto& statement : node->statements)
		statement = checked_cast<IRStatement>(dispatch(statement));

	return node;
}

IRNode* OperatorLoweringPass::visit(IRExpressionStatement* node)
{
	node->expression = checked_cast<IRExpression>(dispatch(node->expression));
	return node;
}

IRNode* OperatorLoweringPass::visit(IRVariableStatement* node)
{
	for (auto& [name, value] : node->items)
		value = checked_cast<IRExpression>(dispatch(value));

	return node;
}

IRNode* OperatorLoweringPass::visit(IRReturnStatement* node)
{
	node->expression = checked_cast<IRExpression>(dispatch(node->expression));
	return node;
}

IRNode* OperatorLoweringPass::visit(IRWhileStatement* node)
{
	node->condition = checked_cast<IRExpression>(dispatch(node->condition));
	node->body = checked_cast<IRStatement>(dispatch(node->body));
	return node;
}

IRNode* OperatorLoweringPass::visit(IRIfStatement* node)
{
	node->condition = checked_cast<IRExpression>(dispatch(node->condition));
	node->ifBody = checked_cast<IRStatement>(dispatch(node->ifBody));
	node->elseBody = (node->elseBody ? checked_cast<IRStatement>(dispatch(node->elseBody)) : nullptr);
	return node;
}

IRNode* OperatorLoweringPass::visit(IRStructDeclaration* node)
{
	return node;
}

IRNode* OperatorLoweringPass::visit(IRFunctionDeclaration* node)
{
	node->body = checked_cast<IRStatement>(dispatch(node->body));
	return node;
}

IRNode* OperatorLoweringPass::visit(IRExternFunctionDeclaration* node)
{
	return node;
}

IRNode* OperatorLoweringPass::visit(IRSourceFile* node)
{
	for (auto& decl : node->declarations)
		decl = checked_cast<IRDeclaration>(dispatch(decl));

	return node;
}