#include "semantic_pass.hpp"

void SemanticPass::analyze(AstNode* program)
{
	visit(program);

	for (auto& [name, structDeclaration] : structs)
	{
		if (typeCtx.structTypes.contains(name))
			return logger.error(structDeclaration, "Struct " + name + " is already defined");

		typeCtx.structTypes.try_emplace(name, new StructType(typeCtx, name, structDeclaration->fields));
	}

	for (auto& [name, function] : functions)
	{
		localVariables.clear();
		for (auto& param : function->parameters)
			localVariables.try_emplace(param.first, param.second);

		functionResult = nullptr;
		expectedFunctionResult = function->result;

		visit(function->body);

		if (!expectedFunctionResult->isVoidType() && !functionResult)
			return logger.error(function, "Missing return statement in function " + name + ", should return " + expectedFunctionResult->toString());
	}
}

Type* SemanticPass::getFunctionResult(std::string const& name, std::vector<Type*> const& args)
{
	if (!functions.contains(name) && !externFunctions.contains(name))
		return nullptr;

	for (auto [it, end] = functions.equal_range(name); it != end; ++it)
	{
		auto function = it->second;
		if (function->parameters.size() != args.size())
			continue;

		for (int i = 0; i < function->parameters.size(); i++)
		{
			if (!Type::areSame(function->parameters[i].second, args[i]))
				break;

			if (i == function->parameters.size() - 1)
				return function->result;
		}
	}

	for (auto [it, end] = externFunctions.equal_range(name); it != end; ++it)
	{
		auto function = it->second;
		if (function->parameters.size() != args.size())
			continue;

		for (int i = 0; i < function->parameters.size(); i++)
		{
			if (!Type::areSame(function->parameters[i].second, args[i]))
				break;

			if (i == function->parameters.size() - 1)
				return function->result;
		}
	}

	return nullptr;
}

Type* SemanticPass::getFunctionResult(std::string const& name, std::vector<Type*> const& args, AstNode* current)
{
	if (!functions.contains(name) && !externFunctions.contains(name))
		return logger.error(current, "No function named " + name, nullptr);

	for (auto [it, end] = functions.equal_range(name); it != end; ++it)
	{
		auto function = it->second;
		if (function->parameters.size() != args.size())
			continue;

		for (int i = 0; i < function->parameters.size(); i++)
		{
			if (!Type::areSame(function->parameters[i].second, args[i]))
				break;

			if (i == function->parameters.size() - 1)
				return function->result;
		}
	}

	for (auto [it, end] = externFunctions.equal_range(name); it != end; ++it)
	{
		auto function = it->second;
		if (function->parameters.size() != args.size())
			continue;

		for (int i = 0; i < function->parameters.size(); i++)
		{
			if (!Type::areSame(function->parameters[i].second, args[i]))
				break;

			if (i == function->parameters.size() - 1)
				return function->result;
		}
	}

	return logger.error(current, "No matching overload for function " + name, nullptr);
}

Type* SemanticPass::visit(IntegerExpression* node)
{
	if (node->suffix == "")
		return (node->computedType = typeCtx.resolveNamedType("i32"));
	else if (node->suffix == "i8")
		return (node->computedType = typeCtx.resolveNamedType("i8"));
	else if (node->suffix == "i16")
		return (node->computedType = typeCtx.resolveNamedType("i16"));
	else if (node->suffix == "i32")
		return (node->computedType = typeCtx.resolveNamedType("i32"));
	else if (node->suffix == "i64")
		return (node->computedType = typeCtx.resolveNamedType("i64"));
	else if (node->suffix == "u8")
		return (node->computedType = typeCtx.resolveNamedType("u8"));
	else if (node->suffix == "u16")
		return (node->computedType = typeCtx.resolveNamedType("u16"));
	else if (node->suffix == "u32")
		return (node->computedType = typeCtx.resolveNamedType("u32"));
	else if (node->suffix == "u64")
		return (node->computedType = typeCtx.resolveNamedType("u64"));
	else
		return logger.error(node, "Invalid integer literal suffix", nullptr);
}

Type* SemanticPass::visit(BoolExpression* node)
{
	return (node->computedType = typeCtx.resolveNamedType("bool"));
}

Type* SemanticPass::visit(CharExpression* node)
{
	return (node->computedType = typeCtx.resolveNamedType("char"));
}

Type* SemanticPass::visit(StringExpression* node)
{
	return (node->computedType = typeCtx.resolveNamedType("str"));
}

Type* SemanticPass::visit(IdentifierExpression* node)
{
	if (!localVariables.contains(node->value))
		return logger.error(node, "Undefined Identifier", nullptr);

	return (node->computedType = localVariables.at(node->value));
}

Type* SemanticPass::visit(StructExpression* node)
{
	if (!typeCtx.structTypes.contains(node->structName))
		return logger.error(node, "Undefined Struct Type", nullptr);

	for (auto& [name, value] : node->fields)
		visit(value);

	auto structType = dynamic_cast<StructType*>(typeCtx.resolveNamedType(node->structName));

	for (auto& [name, value] : node->fields)
	{
		for (int i = 0; i < structType->fields.size(); i++)
		{
			auto& [fieldName, fieldType] = structType->fields[i];
			if (fieldName == name)
			{
				if (fieldType != value->computedType)
					return logger.error(value, "Field " + name + " has type " + fieldType->toString() + ", value type is " + value->computedType->toString(), nullptr);
				break;
			}

			if (i == structType->fields.size() - 1)
				return logger.error(node, "Struct " + structType->name + " does not contain a field called " + name, nullptr);
		}
	}

	for (auto& [fieldName, fieldType] : structType->fields)
	{
		for (int i = 0; i < node->fields.size(); i++)
		{
			auto& [name, value] = node->fields[i];
			if (name == fieldName)
			{
				if (value->computedType != fieldType)
					return logger.error(value, "Field " + name + " has type " + fieldType->toString() + ", value type is " + value->computedType->toString(), nullptr);
				break;
			}

			if (i == node->fields.size() - 1)
				return logger.error(node, "No initializer for field " + fieldName + ": " + fieldType->toString(), nullptr);
		}
	}

	return (node->computedType = structType);
}

Type* SemanticPass::visit(UnaryExpression* node)
{
	auto value = visit(node->expression);
	if (unaryOperators.at(node->type).category == OperatorCategory::UnaryArithmetic && value->isIntegerType())
	{
		return (node->computedType = value);
	}
	else if (unaryOperators.at(node->type).category == OperatorCategory::UnaryBitwise && value->isIntegerType())
	{
		return (node->computedType = value);
	}
	else if (unaryOperators.at(node->type).category == OperatorCategory::UnaryLogic && value->isBoolType())
	{
		return (node->computedType = typeCtx.resolveNamedType("bool"));
	}
	else
	{
		std::vector<Type*> args = std::vector<Type*>({ value });
		auto result = getFunctionResult(unaryOperators.at(node->type).name, args, node);
		return (node->computedType = result);
	}
}

Type* SemanticPass::visit(BinaryExpression* node)
{
	auto left = visit(node->left);
	auto right = visit(node->right);

	if (binaryOperators.at(node->type).category == OperatorCategory::BinaryArithmetic && (left->isIntegerType() && right->isIntegerType()))
	{
		return (node->computedType = ((left->getBitSize() >= right->getBitSize()) ? left : right));
	}
	else if (binaryOperators.at(node->type).category == OperatorCategory::BinaryBitwise && (left->isIntegerType() && right->isIntegerType()) && (left->getBitSize() == right->getBitSize()))
	{
		return (node->computedType = left);
	}
	else if (binaryOperators.at(node->type).category == OperatorCategory::BinaryComparison && (left->isIntegerType() && right->isIntegerType()))
	{
		return (node->computedType = typeCtx.resolveNamedType("bool"));
	}
	else if (binaryOperators.at(node->type).category == OperatorCategory::BinaryLogic && (left->isBoolType() && right->isBoolType()))
	{
		return (node->computedType = typeCtx.resolveNamedType("bool"));
	}
	else if (binaryOperators.at(node->type).category == OperatorCategory::BinaryEquality && (Type::areSame(left, right) || (left->isIntegerType() && right->isIntegerType())))
	{
		return (node->computedType = typeCtx.resolveNamedType("bool"));
	}
	else if (binaryOperators.at(node->type).category == OperatorCategory::BinaryAssign && (Type::areSame(left, right) || (left->isIntegerType() && right->isIntegerType())))
	{
		if (!dynamic_cast<IdentifierExpression*>(node->left))
			return logger.error(node, "Left side of assignment has to be identifier", nullptr);

		if ((left->isIntegerType() && right->isIntegerType()) && (left->getBitSize() < right->getBitSize()))
			logger.warning(node, "Narrowing conversion from " + left->toString() + " to " + right->toString());

		return (node->computedType = left);
	}
	else
	{
		std::vector<Type*> args = std::vector<Type*>({ left, right });
		auto result = getFunctionResult(binaryOperators.at(node->type).name, args, node);

		if (binaryOperators.at(node->type).category == OperatorCategory::BinaryAssign && !Type::areSame(left, result))
			return logger.error(node, "Assignment operator overload function has to return a value that has the type of the left operand", nullptr);

		return (node->computedType = result);
	}
}

Type* SemanticPass::visit(CallExpression* node)
{
	std::vector<Type*> args;
	for (auto arg : node->args)
		args.push_back(visit(arg));

	if (dynamic_cast<IdentifierExpression*>(node->expression))
	{
		auto name = dynamic_cast<IdentifierExpression*>(node->expression)->value;
		auto result = getFunctionResult(name, args, node);
		return (node->computedType = result);
	}
	else
	{
		args.insert(args.begin(), visit(node->expression));
		auto result = getFunctionResult("__call__", args, node);
		return (node->computedType = result);
	}
}

Type* SemanticPass::visit(IndexExpression* node)
{
	std::vector<Type*> args;
	for (auto arg : node->args)
		args.push_back(visit(arg));

	auto value = visit(node->expression);
	if (value->isArrayType() && args.size() == 1 && args.front()->isIntegerType())
	{
		return (node->computedType = dynamic_cast<ArrayType*>(value)->base);
	}
	if (value->isStringType() && args.size() == 1 && args.front()->isIntegerType())
	{
		return (node->computedType = typeCtx.resolveNamedType("u8"));
	}
	else
	{
		args.insert(args.begin(), value);
		return (node->computedType = getFunctionResult("__index__", args, node));
	}
}

Type* SemanticPass::visit(FieldExpression* node)
{
	auto value = visit(node->expression);
	if (!value->isStructType())
		return logger.error(node, "Left side of field expression has to be of struct type", nullptr);

	auto structType = dynamic_cast<StructType*>(value->getResolvedType());
	for (int i = 0; i < structType->fields.size(); i++)
	{
		if (structType->fields[i].first == node->fieldName)
			return (node->computedType = structType->fields[i].second);
	}

	return logger.error(node, "Struct " + structType->name + " does not have a field named " + node->fieldName, nullptr);
}

Type* SemanticPass::visit(BlockStatement* node)
{
	for (auto& statement : node->statements)
	{
		visit(statement);
	}

	return nullptr;
}

Type* SemanticPass::visit(ExpressionStatement* node)
{
	visit(node->expression);
	return nullptr;
}

Type* SemanticPass::visit(VariableStatement* node)
{
	for (auto& [name, value] : node->items)
	{
		if (localVariables.contains(name))
			return logger.error(node, "Variable is already defined", nullptr);

		if (Type::areSame(visit(value), typeCtx.resolveNamedType("void")))
			return logger.error(node, "Variable cannot have void type", nullptr);

		localVariables.try_emplace(name, value->computedType);
	}

	return nullptr;
}

Type* SemanticPass::visit(ReturnStatement* node)
{
	functionResult = visit(node->expression);
	if (!Type::areSame(functionResult, expectedFunctionResult))
	{
		functionResult = nullptr;
		return logger.error(node->expression, "Return expression has to be of function result type", nullptr);
	}

	return nullptr;
}

Type* SemanticPass::visit(WhileStatement* node)
{
	auto condition = visit(node->condition);
	if (!Type::areSame(condition, typeCtx.getNamedType("bool")))
		return logger.error(node->condition, "While condition has to be of boolean type", nullptr);

	auto prevResult = functionResult;
	visit(node->body);
	functionResult = prevResult;

	return nullptr;
}

Type* SemanticPass::visit(IfStatement* node)
{
	auto condition = visit(node->condition);
	if (!Type::areSame(condition, typeCtx.getNamedType("bool")))
		return logger.error(node->condition, "If condition has to be of boolean type", nullptr);

	auto prevResult = functionResult;

	functionResult = nullptr;
	visit(node->ifBody);
	auto ifResult = functionResult;

	functionResult = nullptr;
	if (node->elseBody)
		visit(node->elseBody);
	auto elseResult = functionResult;

	functionResult = (((ifResult != nullptr) && (elseResult != nullptr)) ? ifResult : prevResult);

	return nullptr;
}

Type* SemanticPass::visit(StructDeclaration* node)
{
	if (structs.contains(node->name))
		return logger.error(node, "Struct " + node->name + " is already defined", nullptr);

	structs.try_emplace(node->name, node);
	return nullptr;
}

Type* SemanticPass::visit(FunctionDeclaration* node)
{
	std::vector<Type*> args;
	for (auto& param : node->parameters)
		args.push_back(param.second);

	if (getFunctionResult(node->name, args))
		return logger.error(node, "Function " + node->name + " is already declared with the same parameters", nullptr);

	functions.emplace(node->name, node);
	return nullptr;
}

Type* SemanticPass::visit(ExternFunctionDeclaration* node)
{
	std::vector<Type*> args;
	for (auto& param : node->parameters)
		args.push_back(param.second);

	if (getFunctionResult(node->name, args))
		return logger.error(node, "Function " + node->name + " is already declared with the same parameters", nullptr);

	externFunctions.emplace(node->name, node);
	return nullptr;
}

Type* SemanticPass::visit(Module* node)
{
	for (auto& declaration : node->declarations)
	{
		visit((AstNode*)declaration);
	}

	return nullptr;
}