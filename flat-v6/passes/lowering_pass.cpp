#include "lowering_pass.hpp"

ASTNode* OperatorLoweringPass::process(ASTNode* program)
{
	return dispatch(program);
}

ASTNode* OperatorLoweringPass::visit(ASTIntegerExpression* node)
{
	return node;
}

ASTNode* OperatorLoweringPass::visit(ASTBoolExpression* node)
{
	return node;
}

ASTNode* OperatorLoweringPass::visit(ASTCharExpression* node)
{
	return node;
}

ASTNode* OperatorLoweringPass::visit(ASTStringExpression* node)
{
	return node;
}

ASTNode* OperatorLoweringPass::visit(ASTIdentifierExpression* node)
{
	return node;
}

ASTNode* OperatorLoweringPass::visit(ASTStructExpression* node)
{
	return node;
}

ASTNode* OperatorLoweringPass::visit(ASTUnaryExpression* node)
{
	node->expression = checked_cast<ASTExpression>(dispatch(node->expression));

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
		std::vector<ASTExpression*> args = std::vector<ASTExpression*>({ node->expression });
		auto newNode = astCtx.make<ASTBoundCallExpression>(node->begin, node->end, unaryOperators.at(node->operation).name, args);
		newNode->type = node->type;
		return newNode;
	}
}

ASTNode* OperatorLoweringPass::visit(ASTBinaryExpression* node)
{
	node->left = checked_cast<ASTExpression>(dispatch(node->left));
	node->right = checked_cast<ASTExpression>(dispatch(node->right));

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
			auto wrapArgs = std::vector<ASTExpression*>({ node->left, node->right });
			auto wrappedRight = astCtx.make<ASTBoundCallExpression>(node->begin, node->end, binaryOperators.at(BinaryOperator::Assign).name, wrapArgs);
			auto newNode = astCtx.make<ASTBinaryExpression>(node->begin, node->end, BinaryOperator::Assign, node->left, wrappedRight);
			newNode->type = node->type;
			return newNode;
		}
		else
		{
			auto args = std::vector<ASTExpression*>({ node->left, node->right });
			auto newNode = astCtx.make<ASTBoundCallExpression>(node->begin, node->end, binaryOperators.at(node->operation).name, args);
			newNode->type = node->type;
			return newNode;
		}
	}
}

ASTNode* OperatorLoweringPass::visit(ASTCallExpression* node)
{
	node->expression = checked_cast<ASTExpression>(dispatch(node->expression));
	for (auto& arg : node->args)
		arg = checked_cast<ASTExpression>(dispatch(arg));

	if (dynamic_cast<ASTIdentifierExpression*>(node->expression))
	{
		auto newNode = astCtx.make<ASTBoundCallExpression>(node->begin, node->end, dynamic_cast<ASTIdentifierExpression*>(node->expression)->value, node->args);
		newNode->type = node->type;
		return newNode;
	}
	else
	{
		auto args = node->args;
		args.insert(args.begin(), node->expression);
		auto newNode = astCtx.make<ASTBoundCallExpression>(node->begin, node->end, "__call__", args);
		newNode->type = node->type;
		return newNode;
	}
}

ASTNode* OperatorLoweringPass::visit(ASTIndexExpression* node)
{
	node->expression = checked_cast<ASTExpression>(dispatch(node->expression));
	for (auto& arg : node->args)
		arg = checked_cast<ASTExpression>(dispatch(arg));

	auto value = node->expression->type;
	if (value->isArrayType() && node->args.size() == 1 && node->args.front()->type->isIntegerType())
	{
		auto newNode = astCtx.make<ASTBoundIndexExpression>(node->begin, node->end, node->expression, node->args.front());
		newNode->type = node->type;
		return newNode;
	}
	if (value->isStringType() && node->args.size() == 1 && node->args.front()->type->isIntegerType())
	{
		auto newNode = astCtx.make<ASTBoundIndexExpression>(node->begin, node->end, node->expression, node->args.front());
		newNode->type = node->type;
		return newNode;
	}
	else
	{
		auto args = node->args;
		args.insert(args.begin(), node->expression);
		auto newNode = astCtx.make<ASTBoundCallExpression>(node->begin, node->end, "__index__", args);
		newNode->type = node->type;
		return newNode;
	}
}

ASTNode* OperatorLoweringPass::visit(ASTFieldExpression* node)
{
	node->expression = checked_cast<ASTExpression>(dispatch(node->expression));
	return node;
}

ASTNode* OperatorLoweringPass::visit(ASTBlockStatement* node)
{
	for (auto& statement : node->statements)
		statement = checked_cast<ASTStatement>(dispatch(statement));

	return node;
}

ASTNode* OperatorLoweringPass::visit(ASTExpressionStatement* node)
{
	node->expression = checked_cast<ASTExpression>(dispatch(node->expression));
	return node;
}

ASTNode* OperatorLoweringPass::visit(ASTVariableStatement* node)
{
	for (auto& [name, value] : node->items)
		value = checked_cast<ASTExpression>(dispatch(value));

	return node;
}

ASTNode* OperatorLoweringPass::visit(ASTReturnStatement* node)
{
	node->expression = checked_cast<ASTExpression>(dispatch(node->expression));
	return node;
}

ASTNode* OperatorLoweringPass::visit(ASTWhileStatement* node)
{
	node->condition = checked_cast<ASTExpression>(dispatch(node->condition));
	node->body = checked_cast<ASTStatement>(dispatch(node->body));
	return node;
}

ASTNode* OperatorLoweringPass::visit(ASTIfStatement* node)
{
	node->condition = checked_cast<ASTExpression>(dispatch(node->condition));
	node->ifBody = checked_cast<ASTStatement>(dispatch(node->ifBody));
	node->elseBody = (node->elseBody ? checked_cast<ASTStatement>(dispatch(node->elseBody)) : nullptr);
	return node;
}

ASTNode* OperatorLoweringPass::visit(ASTStructDeclaration* node)
{
	return node;
}

ASTNode* OperatorLoweringPass::visit(ASTFunctionDeclaration* node)
{
	node->body = checked_cast<ASTStatement>(dispatch(node->body));
	return node;
}

ASTNode* OperatorLoweringPass::visit(ASTExternFunctionDeclaration* node)
{
	return node;
}

ASTNode* OperatorLoweringPass::visit(ASTSourceFile* node)
{
	for (auto& decl : node->declarations)
		decl = checked_cast<ASTDeclaration>(dispatch(decl));

	return node;
}