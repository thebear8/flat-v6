#include "codegen_pass.hpp"

#include <llvm/Passes/PassBuilder.h>
#include <llvm/Analysis/LoopAnalysisManager.h>
#include <llvm/Analysis/CGSCCPassManager.h>

void LLVMCodegenPass::compile(AstNode* ast)
{
	isFunctionBodyPass = false;
	dispatch(ast);

	isFunctionBodyPass = true;
	dispatch(ast);
}

void LLVMCodegenPass::optimize()
{
	llvm::LoopAnalysisManager lam;
	llvm::FunctionAnalysisManager fam;
	llvm::CGSCCAnalysisManager cgam;
	llvm::ModuleAnalysisManager mam;

	llvm::PassBuilder pb;
	pb.registerModuleAnalyses(mam);
	pb.registerCGSCCAnalyses(cgam);
	pb.registerFunctionAnalyses(fam);
	pb.registerLoopAnalyses(lam);
	pb.crossRegisterProxies(lam, fam, cgam, mam);

	auto mpm = pb.buildPerModuleDefaultPipeline(llvm::OptimizationLevel::O1);
	mpm.run(mod, mam);
}

llvm::Value* LLVMCodegenPass::visit(IntegerExpression* node)
{
	auto radix = 10;
	auto value = node->value;
	if (value.starts_with("0x"))
	{
		radix = 16;
		value = value.substr(2);
	}
	else if (value.starts_with("0b"))
	{
		radix = 2;
		value = value.substr(2);
	}

	auto type = llvm::IntegerType::get(llvmCtx, node->type->getBitSize());
	return llvm::ConstantInt::get(type, value, radix);
}

llvm::Value* LLVMCodegenPass::visit(BoolExpression* node)
{
	auto value = ((node->value == "true") ? 1 : 0);
	return llvm::ConstantInt::get(llvm::Type::getInt1Ty(llvmCtx), value);
}

llvm::Value* LLVMCodegenPass::visit(CharExpression* node)
{
	size_t position = 0;
	return llvm::ConstantInt::get(llvm::Type::getInt32Ty(llvmCtx), unescapeCodePoint(node->value, position, node));
}

llvm::Value* LLVMCodegenPass::visit(StringExpression* node)
{
	std::vector<llvm::Constant*> stringBytes;
	for (auto c : unescapeString(node->value, node))
		stringBytes.push_back(llvm::ConstantInt::get(llvm::Type::getInt8Ty(llvmCtx), c));

	auto fieldTypes = std::vector<llvm::Type*>({
		llvm::Type::getInt64Ty(llvmCtx),
		llvm::ArrayType::get(llvm::Type::getInt8Ty(llvmCtx), stringBytes.size())
		});
	auto structType = llvm::StructType::get(llvmCtx, fieldTypes);

	auto fieldValues = std::vector<llvm::Constant*>({
		llvm::ConstantInt::get(llvm::Type::getInt64Ty(llvmCtx), stringBytes.size()),
		llvm::ConstantArray::get(llvm::ArrayType::get(llvm::Type::getInt8Ty(llvmCtx), stringBytes.size()), stringBytes)
		});
	auto structValue = llvm::ConstantStruct::get(structType, fieldValues);

	return new llvm::GlobalVariable(mod, structType, true, llvm::GlobalValue::LinkageTypes::InternalLinkage, structValue);
}

llvm::Value* LLVMCodegenPass::visit(IdentifierExpression* node)
{
	if (!localValues.contains(node->value))
		return logger.error(node, "Undefined local variable " + node->value, nullptr);

	return builder.CreateLoad(getLLVMType(node->type), localValues.at(node->value), node->value + "_");
}

llvm::Value* LLVMCodegenPass::visit(StructExpression* node)
{
	auto type = dynamic_cast<StructType*>(node->type);
	auto structPtr = builder.CreateAlloca(getLLVMType(type), nullptr, type->name + "_");

	for (int i = 0; i < type->fields.size(); i++)
	{
		for (int j = 0; j < node->fields.size(); j++)
		{
			if (node->fields.at(j).first == type->fields.at(i).first)
			{
				auto fieldName = node->fields.at(j).first;
				auto fieldValue = dispatch(node->fields.at(j).second);
				auto fieldPtr = builder.CreateStructGEP(getLLVMType(type), structPtr, i, type->name + "." + fieldName + "_");
				builder.CreateStore(fieldValue, fieldPtr);
				break;
			}

			if (j == node->fields.size())
				return logger.error(node, "No initializer for field " + type->fields.at(i).first, nullptr);
		}
	}

	return builder.CreateLoad(getLLVMType(type), structPtr);
}

llvm::Value* LLVMCodegenPass::visit(UnaryExpression* node)
{
	if (node->operation == UnaryOperator::Positive)
	{
		return dispatch(node->expression);
	}
	else if (node->operation == UnaryOperator::Negative)
	{
		return builder.CreateNeg(dispatch(node->expression));
	}
	else if (node->operation == UnaryOperator::BitwiseNot)
	{
		return builder.CreateNot(dispatch(node->expression));
	}
	else if (node->operation == UnaryOperator::LogicalNot)
	{
		return builder.CreateNot(dispatch(node->expression));
	}
	else
	{
		return logger.error(node, "Invalid Unary operator", nullptr);
	}
}

llvm::Value* LLVMCodegenPass::visit(BinaryExpression* node)
{
	if (dynamic_cast<IdentifierExpression*>(node->left))
	{
		auto name = dynamic_cast<IdentifierExpression*>(node->left)->value;
		if (!localValues.contains(name))
			return logger.error(node, "Undefined local variable " + name, nullptr);

		builder.CreateStore(dispatch(node->right), localValues.at(name));
		return builder.CreateLoad(getLLVMType(node->type), localValues.at(name), name + "_");
	}
	else
	{
		auto left = dispatch(node->left);
		auto right = ((node->left->type->isSigned()) ?
			builder.CreateSExtOrTrunc(dispatch(node->right), getLLVMType(node->left->type)) :
			builder.CreateZExtOrTrunc(dispatch(node->right), getLLVMType(node->left->type)));

		if (node->operation == BinaryOperator::Add)
		{
			return builder.CreateAdd(left, right);
		}
		else if (node->operation == BinaryOperator::Subtract)
		{
			return builder.CreateSub(left, right);
		}
		else if (node->operation == BinaryOperator::Multiply)
		{
			return builder.CreateMul(left, right);
		}
		else if (node->operation == BinaryOperator::Divide)
		{
			return (node->left->type->isSigned() ?
				builder.CreateSDiv(left, right) :
				builder.CreateUDiv(left, right));
		}
		else if (node->operation == BinaryOperator::Modulo)
		{
			return (node->left->type->isSigned() ?
				builder.CreateSRem(left, right) :
				builder.CreateURem(left, right));
		}
		else if (node->operation == BinaryOperator::BitwiseAnd)
		{
			return builder.CreateAnd(left, right);
		}
		else if (node->operation == BinaryOperator::BitwiseOr)
		{
			return builder.CreateOr(left, right);
		}
		else if (node->operation == BinaryOperator::BitwiseXor)
		{
			return builder.CreateXor(left, right);
		}
		else if (node->operation == BinaryOperator::ShiftLeft)
		{
			return builder.CreateShl(left, right);
		}
		else if (node->operation == BinaryOperator::ShiftRight)
		{
			return builder.CreateAShr(left, right);
		}
		else if (node->operation == BinaryOperator::LogicalAnd)
		{
			return builder.CreateLogicalAnd(left, right);
		}
		else if (node->operation == BinaryOperator::LogicalOr)
		{
			return builder.CreateLogicalOr(left, right);
		}
		else if (node->operation == BinaryOperator::Equal)
		{
			return builder.CreateCmp(llvm::CmpInst::Predicate::ICMP_EQ, left, right);
		}
		else if (node->operation == BinaryOperator::NotEqual)
		{
			return builder.CreateCmp(llvm::CmpInst::Predicate::ICMP_NE, left, right);
		}
		else if (node->operation == BinaryOperator::LessThan)
		{
			return (node->left->type->isSigned() ?
				builder.CreateCmp(llvm::CmpInst::Predicate::ICMP_SLT, left, right) :
				builder.CreateCmp(llvm::CmpInst::Predicate::ICMP_ULT, left, right));
		}
		else if (node->operation == BinaryOperator::GreaterThan)
		{
			return (node->left->type->isSigned() ?
				builder.CreateCmp(llvm::CmpInst::Predicate::ICMP_SGT, left, right) :
				builder.CreateCmp(llvm::CmpInst::Predicate::ICMP_UGT, left, right));
		}
		else if (node->operation == BinaryOperator::LessOrEqual)
		{
			return (node->left->type->isSigned() ?
				builder.CreateCmp(llvm::CmpInst::Predicate::ICMP_SLE, left, right) :
				builder.CreateCmp(llvm::CmpInst::Predicate::ICMP_ULE, left, right));
		}
		else if (node->operation == BinaryOperator::GreaterOrEqual)
		{
			return (node->left->type->isSigned() ?
				builder.CreateCmp(llvm::CmpInst::Predicate::ICMP_SGE, left, right) :
				builder.CreateCmp(llvm::CmpInst::Predicate::ICMP_UGE, left, right));
		}
		else
		{
			return logger.error(node, "Invalid binary expression", nullptr);
		}
	}
}

llvm::Value* LLVMCodegenPass::visit(BoundCallExpression* node)
{
	std::vector<Type*> argTypes;
	for (auto& arg : node->args)
		argTypes.push_back(arg->type);

	std::vector<llvm::Type*> llvmArgTypes;
	for (auto& arg : node->args)
		llvmArgTypes.push_back(getLLVMType(arg->type));

	std::vector<llvm::Value*> argValues;
	for (auto& arg : node->args)
		argValues.push_back(dispatch(arg));

	auto name = getMangledFunction(node->identifier, argTypes);
	auto type = llvm::FunctionType::get(getLLVMType(node->type), llvmArgTypes, false);
	auto function = (mod.getFunction(name) ? mod.getFunction(name) : mod.getFunction(node->identifier));
	if (!function)
		return logger.error(node, "No matching function named " + name + " found", nullptr);

	if (function->getFunctionType() != type)
		return logger.error(node, "Type of function " + name + " does not match", nullptr);

	return builder.CreateCall(function, argValues);
}

llvm::Value* LLVMCodegenPass::visit(BoundIndexExpression* node)
{
	auto fieldTypes = std::vector<llvm::Type*>({
		llvm::Type::getInt64Ty(llvmCtx),
		llvm::ArrayType::get(getLLVMType(node->type), 0)
		});
	auto arrayType = llvm::StructType::get(llvmCtx, fieldTypes);

	auto indexes = std::vector<llvm::Value*>({
		llvm::ConstantInt::get(llvm::Type::getInt64Ty(llvmCtx), 0),
		llvm::ConstantInt::get(llvm::Type::getInt32Ty(llvmCtx), 1),
		dispatch(node->index)
		});

	auto ptr = builder.CreateGEP(arrayType, dispatch(node->expression), indexes);
	return builder.CreateLoad(getLLVMType(node->type), ptr);
}

llvm::Value* LLVMCodegenPass::visit(FieldExpression* node)
{
	auto structType = dynamic_cast<StructType*>(node->expression->type);
	for (int i = 0; i < structType->fields.size(); i++)
	{
		if (structType->fields[i].first == node->fieldName)
		{
			auto value = builder.CreateAlloca(getLLVMType(structType), nullptr, structType->name + "_");
			builder.CreateStore(dispatch(node->expression), value);
			auto ptr = builder.CreateStructGEP(getLLVMType(structType), value, i);
			return builder.CreateLoad(getLLVMType(node->type), ptr, structType->toString() + "." + node->fieldName + "_");
		}
	}

	return logger.error(node, "Struct " + structType->toString() + " does not have a field called " + node->fieldName, nullptr);
}

llvm::Value* LLVMCodegenPass::visit(BlockStatement* node)
{
	auto prevLocalValues = localValues;

	for (auto& statement : node->statements)
		dispatch(statement);

	localValues = prevLocalValues;

	return nullptr;
}

llvm::Value* LLVMCodegenPass::visit(ExpressionStatement* node)
{
	dispatch(node->expression);
	return nullptr;
}

llvm::Value* LLVMCodegenPass::visit(VariableStatement* node)
{
	for (auto& [name, value] : node->items)
	{
		if (localValues.contains(name))
			return logger.error(node, "Variable " + name + " is already defined", nullptr);

		localValues.try_emplace(name, builder.CreateAlloca(getLLVMType(value->type), nullptr, name));
		builder.CreateStore(dispatch(value), localValues.at(name));
	}

	return nullptr;
}

llvm::Value* LLVMCodegenPass::visit(ReturnStatement* node)
{
	if (node->expression)
		builder.CreateRet(dispatch(node->expression));
	else
		builder.CreateRetVoid();

	return nullptr;
}

llvm::Value* LLVMCodegenPass::visit(WhileStatement* node)
{
	auto parentFunction = builder.GetInsertBlock()->getParent();

	auto conditionBlock = llvm::BasicBlock::Create(llvmCtx, "while_cond_block");
	auto bodyBlock = llvm::BasicBlock::Create(llvmCtx, "while_body_block");
	auto endBlock = llvm::BasicBlock::Create(llvmCtx, "while_end_block");

	builder.CreateBr(conditionBlock);

	parentFunction->getBasicBlockList().push_back(conditionBlock);
	builder.SetInsertPoint(conditionBlock);
	builder.CreateCondBr(dispatch(node->condition), bodyBlock, endBlock);

	parentFunction->getBasicBlockList().push_back(bodyBlock);
	builder.SetInsertPoint(bodyBlock);
	dispatch(node->body);
	builder.CreateBr(conditionBlock);

	parentFunction->getBasicBlockList().push_back(endBlock);
	builder.SetInsertPoint(endBlock);

	return nullptr;
}

llvm::Value* LLVMCodegenPass::visit(IfStatement* node)
{
	auto hasElse = (node->elseBody != nullptr);
	auto parentFunction = builder.GetInsertBlock()->getParent();

	auto ifBlock = llvm::BasicBlock::Create(llvmCtx, "if_then_block");
	auto elseBlock = (hasElse ? llvm::BasicBlock::Create(llvmCtx, "if_else_block") : nullptr);
	auto endBlock = llvm::BasicBlock::Create(llvmCtx, "if_end_block");

	builder.CreateCondBr(dispatch(node->condition), ifBlock, (hasElse ? elseBlock : endBlock));

	parentFunction->getBasicBlockList().push_back(ifBlock);
	builder.SetInsertPoint(ifBlock);
	dispatch(node->ifBody);
	builder.CreateBr(endBlock);

	if (hasElse)
	{
		parentFunction->getBasicBlockList().push_back(elseBlock);
		builder.SetInsertPoint(elseBlock);
		dispatch(node->elseBody);
		builder.CreateBr(endBlock);
	}

	parentFunction->getBasicBlockList().push_back(endBlock);
	builder.SetInsertPoint(endBlock);

	return nullptr;
}

llvm::Value* LLVMCodegenPass::visit(StructDeclaration* node)
{
	return nullptr;
}

llvm::Value* LLVMCodegenPass::visit(FunctionDeclaration* node)
{
	std::vector<Type*> params;
	for (auto& [name, type] : node->parameters)
		params.push_back(type);

	std::vector<llvm::Type*> llvmParams;
	for (auto& param : params)
		llvmParams.push_back(getLLVMType(param));

	auto name = getMangledFunction(node->name, params);
	auto type = llvm::FunctionType::get(getLLVMType(node->result), llvmParams, false);

	if (!isFunctionBodyPass)
	{
		llvm::Function::Create(type, llvm::GlobalValue::LinkageTypes::ExternalLinkage, name, mod);
	}
	else
	{
		auto function = mod.getFunction(name);
		if (!function)
			return logger.error(node, "Function " + name + " is undefined", nullptr);

		auto entryBlock = llvm::BasicBlock::Create(llvmCtx, "entry");
		auto bodyBlock = llvm::BasicBlock::Create(llvmCtx, "body");

		function->getBasicBlockList().push_back(entryBlock);
		builder.SetInsertPoint(entryBlock);

		localValues.clear();
		for (int i = 0; i < node->parameters.size(); i++)
		{
			auto& [name, type] = node->parameters.at(i);
			if (localValues.contains(name))
				return logger.error(node, "Parameter " + name + " is already defined", nullptr);

			localValues.try_emplace(name, builder.CreateAlloca(getLLVMType(type), nullptr, name + "_"));
			builder.CreateStore(function->getArg(i), localValues.at(name));
		}

		builder.CreateBr(bodyBlock);
		function->getBasicBlockList().push_back(bodyBlock);
		builder.SetInsertPoint(bodyBlock);
		dispatch(node->body);

		if (node->result->isVoidType() && !builder.GetInsertBlock()->getTerminator())
			builder.CreateRetVoid();
	}

	return nullptr;
}

llvm::Value* LLVMCodegenPass::visit(ExternFunctionDeclaration* node)
{
	if (!isFunctionBodyPass)
	{
		std::vector<llvm::Type*> llvmParams;
		for (auto& [name, type] : node->parameters)
			llvmParams.push_back(getLLVMType(type));

		auto type = llvm::FunctionType::get(getLLVMType(node->result), llvmParams, false);
		auto function = llvm::Function::Create(type, llvm::GlobalValue::LinkageTypes::ExternalLinkage, node->name, mod);
		function->setCallingConv(llvm::CallingConv::Win64);
		function->setDLLStorageClass(llvm::GlobalValue::DLLStorageClassTypes::DLLImportStorageClass);
	}

	return nullptr;
}

llvm::Value* LLVMCodegenPass::visit(ParsedSourceFile* node)
{
	for (auto& decl : node->structs)
		dispatch(decl);

	for (auto& decl : node->externFunctions)
		dispatch(decl);

	for (auto& decl : node->functions)
		dispatch(decl);

	return nullptr;
}

llvm::Type* LLVMCodegenPass::getLLVMType(Type* type)
{
	if (llvmTypes.contains(type))
	{
		return llvmTypes.at(type);
	}
	else if (type->isVoidType())
	{
		llvmTypes.try_emplace(type, llvm::Type::getVoidTy(llvmCtx));
		return llvmTypes.at(type);
	}
	else if (type->isBoolType())
	{
		llvmTypes.try_emplace(type, llvm::Type::getInt1Ty(llvmCtx));
		return llvmTypes.at(type);
	}
	else if (type->isIntegerType())
	{
		llvmTypes.try_emplace(type, llvm::Type::getIntNTy(llvmCtx, type->getBitSize()));
		return llvmTypes.at(type);
	}
	else if (type->isCharType())
	{
		llvmTypes.try_emplace(type, llvm::Type::getInt32Ty(llvmCtx));
		return llvmTypes.at(type);
	}
	else if (type->isStringType())
	{
		llvmTypes.try_emplace(type, getLLVMType(typeCtx.getArrayType(typeCtx.getResolvedType("u8"))));
		return llvmTypes.at(type);
	}
	else if (type->isStructType())
	{
		auto structType = dynamic_cast<StructType*>(type);

		std::vector<llvm::Type*> fields;
		for (auto& [fieldName, fieldType] : structType->fields)
			fields.push_back(getLLVMType(fieldType));

		llvmTypes.try_emplace(type, llvm::StructType::get(llvmCtx, fields));
		return llvmTypes.at(type);
	}
	else if (type->isPointerType())
	{
		auto base = dynamic_cast<PointerType*>(type)->base;
		llvmTypes.try_emplace(type, getLLVMType(base)->getPointerTo());
		return llvmTypes.at(type);
	}
	else if (type->isArrayType())
	{
		auto base = dynamic_cast<ArrayType*>(type)->base;
		auto fields = std::vector<llvm::Type*>({
			llvm::Type::getInt64Ty(llvmCtx),
			llvm::ArrayType::get(getLLVMType(base), 0)
			});

		llvmTypes.try_emplace(type, llvm::StructType::get(llvmCtx, fields)->getPointerTo());
		return llvmTypes.at(type);
	}
	else
	{
		throw std::exception();
	}
}

std::string LLVMCodegenPass::getMangledType(Type* type)
{
	if (type->isVoidType())
	{
		return "V";
	}
	else if (type->isBoolType())
	{
		return "B";
	}
	else if (type->isIntegerType())
	{
		return (type->isSigned() ? "I" : "U") + std::to_string(type->getBitSize());
	}
	else if (type->isCharType())
	{
		return "C";
	}
	else if (type->isStringType())
	{
		return "Str";
	}
	else if (type->isStructType())
	{
		auto structType = dynamic_cast<StructType*>(type);
		auto output = "S_" + structType->name + "_";

		for (auto& [fieldName, fieldType] : structType->fields)
			output += getMangledType(fieldType);

		output += "_";
		return output;
	}
	else if (type->isPointerType())
	{
		auto ptrType = dynamic_cast<PointerType*>(type);
		return "P_" + getMangledType(ptrType->base) + "_";
	}
	else if (type->isArrayType())
	{
		auto arrType = dynamic_cast<ArrayType*>(type);
		return "A_" + getMangledType(arrType->base) + "_";
	}
	else
	{
		throw std::exception();
	}
}

std::string LLVMCodegenPass::getMangledFunction(std::string const& function, std::vector<Type*> const& params)
{
	auto output = function + "@";
	for (auto& param : params)
		output += getMangledType(param);

	return output;
}

std::string LLVMCodegenPass::unescapeString(std::string const& input, AstNode* node)
{
	std::string output;

	size_t position = 0;
	while (position < input.length())
	{
		uint32_t cp = unescapeCodePoint(input, position, node);
		if (cp < 0x7F)
		{
			output += (char)cp;
		}
		else if (cp <= 0x07FF)
		{
			output += (char)(((cp >> 6) & 0x1F) | 0xC0);
			output += (char)(((cp >> 0) & 0x3F) | 0x80);
		}
		else if (cp <= 0xFFFF)
		{
			output += (char)(((cp >> 12) & 0x0F) | 0xE0);
			output += (char)(((cp >> 6) & 0x3F) | 0x80);
			output += (char)(((cp >> 0) & 0x3F) | 0x80);
		}
		else if (cp <= 0x10FFFF)
		{
			output += (char)(((cp >> 18) & 0x07) | 0xF0);
			output += (char)(((cp >> 12) & 0x3F) | 0x80);
			output += (char)(((cp >> 6) & 0x3F) | 0x80);
			output += (char)(((cp >> 0) & 0x3F) | 0x80);
		}
		else
		{
			return logger.error(node, "Invalid Unicode code point", "");
		}
	}

	output += (char)0;
	return output;
}

uint32_t LLVMCodegenPass::unescapeCodePoint(std::string const& input, size_t& position, AstNode* node)
{
	if (position < input.length() && input[position] == '\\')
	{
		position++;
		if (position < input.length() && isDigit(input[position])) // octal char literal
		{
			size_t start = position;
			while (position < input.length() && isDigit(position))
				position++;

			if ((position - start) > 3)
				return logger.error(node, "Octal char literal cannot have more than three digits", 0);

			return std::stoul(input.substr(start, (position - start)), nullptr, 8);
		}
		else if (position < input.length() && input[position] == 'x') // hex char literal
		{
			position++;
			size_t start = position;
			while (position < input.length() && isDigit(input[position]))
				position++;

			if ((position - start) == 0)
				return logger.error(node, "Hex char literal cannot have zero digits", 0);

			return std::stoul(input.substr(start, (position - start)), nullptr, 16);
		}
		else if (position < input.length() && input[position] == 'u') // 0xhhhh unicode code point
		{
			position++;
			size_t start = position;
			while (position < input.length() && isDigit(input[position]))
				position++;

			if ((position - start) != 4)
				logger.error(node, "2 byte Unicode code point (\\u) must have 4 digits", "");

			return std::stoul(input.substr(start, (position - start)), nullptr, 16);
		}
		else if (position < input.length() && input[position] == 'U') // 0xhhhhhhhh unicode code point
		{
			position++;
			size_t start = position;
			while (position < input.length() && isDigit(input[position]))
				position++;

			if ((position - start) != 8)
				logger.error(node, "4 byte Unicode code point (\\U) must have 8 digits", "");

			return std::stoul(input.substr(start, (position - start)), nullptr, 16);
		}
		else if (position < input.length())
		{
			if (!escapeChars.contains(input[position]))
				return logger.error(node, "Invalid escape sequence", 0);

			position++;
			return escapeChars.at(input[position]);
		}
		else
		{
			return logger.error(node, "Incomplete escape sequence", 0);
		}
	}
	else if (position < input.length())
	{
		return input[position++];
	}
	else
	{
		return logger.error(node, "Unexpected end of char sequence", 0);
	}
}