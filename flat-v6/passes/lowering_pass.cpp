#include "lowering_pass.hpp"

AstNode* OperatorLoweringPass::process(AstNode* program)
{
	return visit(program);
}

AstNode* OperatorLoweringPass::visit(IntegerExpression* node)
{
	return node;
}

AstNode* OperatorLoweringPass::visit(BoolExpression* node)
{
	return node;
}

AstNode* OperatorLoweringPass::visit(CharExpression* node)
{
	return node;
}

AstNode* OperatorLoweringPass::visit(StringExpression* node)
{
	return node;
}

AstNode* OperatorLoweringPass::visit(IdentifierExpression* node)
{
	return node;
}

AstNode* OperatorLoweringPass::visit(StructExpression* node)
{
	return node;
}

AstNode* OperatorLoweringPass::visit(UnaryExpression* node)
{
	node->expression = checked_cast<Expression>(visit(node->expression));

	auto value = node->expression->computedType;
	if (unaryOperators.at(node->type).category == OperatorCategory::UnaryArithmetic && value->isIntegerType())
	{
		return node;
	}
	else if (unaryOperators.at(node->type).category == OperatorCategory::UnaryBitwise && value->isIntegerType())
	{
		return node;
	}
	else if (unaryOperators.at(node->type).category == OperatorCategory::UnaryLogic && value->isBoolType())
	{
		return node;
	}
	else
	{
		std::vector<Expression*> args = std::vector<Expression*>({ node->expression });
		auto newNode = astCtx.make<BoundCallExpression>(node->begin, node->end, unaryOperators.at(node->type).name, args);
		newNode->computedType = node->computedType;
		return newNode;
	}
}

AstNode* OperatorLoweringPass::visit(BinaryExpression* node)
{
	node->left = checked_cast<Expression>(visit(node->left));
	node->right = checked_cast<Expression>(visit(node->right));

	auto left = node->left->computedType;
	auto right = node->right->computedType;

	if (binaryOperators.at(node->type).category == OperatorCategory::BinaryArithmetic && (left->isIntegerType() && right->isIntegerType()))
	{
		return node;
	}
	else if (binaryOperators.at(node->type).category == OperatorCategory::BinaryBitwise && (left->isIntegerType() && right->isIntegerType()) && (left->getBitSize() == right->getBitSize()))
	{
		return node;
	}
	else if (binaryOperators.at(node->type).category == OperatorCategory::BinaryComparison && (left->isIntegerType() && right->isIntegerType()))
	{
		return node;
	}
	else if (binaryOperators.at(node->type).category == OperatorCategory::BinaryLogic && (left->isBoolType() && right->isBoolType()))
	{
		return node;
	}
	else if (binaryOperators.at(node->type).category == OperatorCategory::BinaryEquality && ((left == right) || (left->isIntegerType() && right->isIntegerType())))
	{
		return node;
	}
	else if (binaryOperators.at(node->type).category == OperatorCategory::BinaryAssign && ((left == right) || (left->isIntegerType() && right->isIntegerType())))
	{
		return node;
	}
	else
	{
		if (node->type == BinaryOperator::Assign)
		{
			auto wrapArgs = std::vector<Expression*>({ node->left, node->right });
			auto wrappedRight = astCtx.make<BoundCallExpression>(node->begin, node->end, binaryOperators.at(BinaryOperator::Assign).name, wrapArgs);
			auto newNode = astCtx.make<BinaryExpression>(node->begin, node->end, BinaryOperator::Assign, node->left, wrappedRight);
			newNode->computedType = node->computedType;
			return newNode;
		}
		else
		{
			auto args = std::vector<Expression*>({ node->left, node->right });
			auto newNode = astCtx.make<BoundCallExpression>(node->begin, node->end, binaryOperators.at(node->type).name, args);
			newNode->computedType = node->computedType;
			return newNode;
		}
	}
}

AstNode* OperatorLoweringPass::visit(CallExpression* node)
{
	node->expression = checked_cast<Expression>(visit(node->expression));
	for (auto& arg : node->args)
		arg = checked_cast<Expression>(visit(arg));

	if (dynamic_cast<IdentifierExpression*>(node->expression))
	{
		auto newNode = astCtx.make<BoundCallExpression>(node->begin, node->end, dynamic_cast<IdentifierExpression*>(node->expression)->value, node->args);
		newNode->computedType = node->computedType;
		return newNode;
	}
	else
	{
		auto args = node->args;
		args.insert(args.begin(), node->expression);
		auto newNode = astCtx.make<BoundCallExpression>(node->begin, node->end, "__call__", args);
		newNode->computedType = node->computedType;
		return newNode;
	}
}

AstNode* OperatorLoweringPass::visit(IndexExpression* node)
{
	node->expression = checked_cast<Expression>(visit(node->expression));
	for (auto& arg : node->args)
		arg = checked_cast<Expression>(visit(arg));

	auto value = node->expression->computedType;
	if (value->isArrayType() && node->args.size() == 1 && node->args.front()->computedType->isIntegerType())
	{
		auto newNode = astCtx.make<BoundIndexExpression>(node->begin, node->end, node->expression, node->args.front());
		newNode->computedType = node->computedType;
		return newNode;
	}
	if (value->isStringType() && node->args.size() == 1 && node->args.front()->computedType->isIntegerType())
	{
		auto newNode = astCtx.make<BoundIndexExpression>(node->begin, node->end, node->expression, node->args.front());
		newNode->computedType = node->computedType;
		return newNode;
	}
	else
	{
		auto args = node->args;
		args.insert(args.begin(), node->expression);
		auto newNode = astCtx.make<BoundCallExpression>(node->begin, node->end, "__index__", args);
		newNode->computedType = node->computedType;
		return newNode;
	}
}

AstNode* OperatorLoweringPass::visit(FieldExpression* node)
{
	node->expression = checked_cast<Expression>(visit(node->expression));
	return node;
}

AstNode* OperatorLoweringPass::visit(BlockStatement* node)
{
	for (auto& statement : node->statements)
		statement = checked_cast<Statement>(visit(statement));

	return node;
}

AstNode* OperatorLoweringPass::visit(ExpressionStatement* node)
{
	node->expression = checked_cast<Expression>(visit(node->expression));
	return node;
}

AstNode* OperatorLoweringPass::visit(VariableStatement* node)
{
	for (auto& [name, value] : node->items)
		value = checked_cast<Expression>(visit(value));

	return node;
}

AstNode* OperatorLoweringPass::visit(ReturnStatement* node)
{
	node->expression = checked_cast<Expression>(visit(node->expression));
	return node;
}

AstNode* OperatorLoweringPass::visit(WhileStatement* node)
{
	node->condition = checked_cast<Expression>(visit(node->condition));
	node->body = checked_cast<Statement>(visit(node->body));
	return node;
}

AstNode* OperatorLoweringPass::visit(IfStatement* node)
{
	node->condition = checked_cast<Expression>(visit(node->condition));
	node->ifBody = checked_cast<Statement>(visit(node->ifBody));
	node->elseBody = (node->elseBody ? checked_cast<Statement>(visit(node->elseBody)) : nullptr);
	return node;
}

AstNode* OperatorLoweringPass::visit(StructDeclaration* node)
{
	return node;
}

AstNode* OperatorLoweringPass::visit(FunctionDeclaration* node)
{
	node->body = checked_cast<Statement>(visit(node->body));
	return node;
}

AstNode* OperatorLoweringPass::visit(ExternFunctionDeclaration* node)
{
	return node;
}

AstNode* OperatorLoweringPass::visit(Module* node)
{
	for (auto& declaration : node->declarations)
		declaration = checked_cast<Declaration>(visit(declaration));

	return node;
}