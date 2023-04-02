#include "codegen_pass.hpp"

#include <llvm/Analysis/CGSCCPassManager.h>
#include <llvm/Analysis/LoopAnalysisManager.h>
#include <llvm/Passes/PassBuilder.h>

void LLVMCodegenPass::process(IRModule* node)
{
    dispatch(node);
}

llvm::Value* LLVMCodegenPass::visit(IRIntegerExpression* node)
{
    auto type = llvm::IntegerType::get(m_llvmCtx, (unsigned int)node->width);
    return llvm::ConstantInt::get(type, node->value, (uint8_t)node->radix);
}

llvm::Value* LLVMCodegenPass::visit(IRBoolExpression* node)
{
    return llvm::ConstantInt::get(
        llvm::Type::getInt1Ty(m_llvmCtx), node->value
    );
}

llvm::Value* LLVMCodegenPass::visit(IRCharExpression* node)
{
    return llvm::ConstantInt::get(
        llvm::Type::getInt32Ty(m_llvmCtx), node->value
    );
}

llvm::Value* LLVMCodegenPass::visit(IRStringExpression* node)
{
    std::vector<llvm::Constant*> stringBytes;
    for (auto c : node->value)
        stringBytes.push_back(
            llvm::ConstantInt::get(llvm::Type::getInt8Ty(m_llvmCtx), c)
        );

    auto fieldTypes = std::vector<llvm::Type*>(
        { llvm::Type::getInt64Ty(m_llvmCtx),
          llvm::ArrayType::get(
              llvm::Type::getInt8Ty(m_llvmCtx), stringBytes.size()
          ) }
    );
    auto structType = llvm::StructType::get(m_llvmCtx, fieldTypes);

    auto fieldValues = std::vector<llvm::Constant*>(
        { llvm::ConstantInt::get(
              llvm::Type::getInt64Ty(m_llvmCtx), stringBytes.size()
          ),
          llvm::ConstantArray::get(
              llvm::ArrayType::get(
                  llvm::Type::getInt8Ty(m_llvmCtx), stringBytes.size()
              ),
              stringBytes
          ) }
    );
    auto structValue = llvm::ConstantStruct::get(structType, fieldValues);

    return new llvm::GlobalVariable(
        m_llvmModule,
        structType,
        true,
        llvm::GlobalValue::LinkageTypes::InternalLinkage,
        structValue
    );
}

llvm::Value* LLVMCodegenPass::visit(IRIdentifierExpression* node)
{
    assert(
        m_localValues.contains(node->value)
        && "Undefined local Variable in identifier expression"
    );
    return m_builder.CreateLoad(
        getLLVMType(node->getType()),
        m_localValues.at(node->value),
        node->value + "_"
    );
}

llvm::Value* LLVMCodegenPass::visit(IRStructExpression* node)
{
    auto type = dynamic_cast<IRStructType*>(node->getType());
    auto structPtr =
        m_builder.CreateAlloca(getLLVMType(type), nullptr, type->name + "_");

    for (int i = 0; i < type->fields.size(); i++)
    {
        for (int j = 0; j < node->fields.size(); j++)
        {
            if (node->fields.at(j).first == type->fields.at(i).first)
            {
                auto const& fieldName = node->fields.at(j).first;
                auto fieldValue = dispatch(node->fields.at(j).second);
                auto fieldPtr = m_builder.CreateStructGEP(
                    getLLVMType(type),
                    structPtr,
                    i,
                    type->name + "." + fieldName + "_"
                );
                m_builder.CreateStore(fieldValue, fieldPtr);
                break;
            }

            if (j == node->fields.size())
                assert(0 && "No initializer for field in struct expression");
        }
    }

    return m_builder.CreateLoad(getLLVMType(type), structPtr);
}

llvm::Value* LLVMCodegenPass::visit(IRUnaryExpression* node)
{
    if (node->operation == UnaryOperator::Positive)
    {
        return dispatch(node->expression);
    }
    else if (node->operation == UnaryOperator::Negative)
    {
        return m_builder.CreateNeg(dispatch(node->expression));
    }
    else if (node->operation == UnaryOperator::BitwiseNot)
    {
        return m_builder.CreateNot(dispatch(node->expression));
    }
    else if (node->operation == UnaryOperator::LogicalNot)
    {
        return m_builder.CreateNot(dispatch(node->expression));
    }
    else
    {
        assert(0 && "Invalid operator in unary expression");
        return nullptr;
    }
}

llvm::Value* LLVMCodegenPass::visit(IRBinaryExpression* node)
{
    if (dynamic_cast<IRIdentifierExpression*>(node->left))
    {
        auto const& name = ((IRIdentifierExpression*)node->left)->value;
        assert(m_localValues.contains(name) && "Undefined local variable");

        m_builder.CreateStore(dispatch(node->right), m_localValues.at(name));
        return m_builder.CreateLoad(
            getLLVMType(node->getType()), m_localValues.at(name), name + "_"
        );
    }
    else
    {
        auto left = dispatch(node->left);
        auto right =
            ((node->left->getType()->isSigned())
                 ? m_builder.CreateSExtOrTrunc(
                     dispatch(node->right), getLLVMType(node->left->getType())
                 )
                 : m_builder.CreateZExtOrTrunc(
                     dispatch(node->right), getLLVMType(node->left->getType())
                 ));

        if (node->operation == BinaryOperator::Add)
        {
            return m_builder.CreateAdd(left, right);
        }
        else if (node->operation == BinaryOperator::Subtract)
        {
            return m_builder.CreateSub(left, right);
        }
        else if (node->operation == BinaryOperator::Multiply)
        {
            return m_builder.CreateMul(left, right);
        }
        else if (node->operation == BinaryOperator::Divide)
        {
            return (
                node->left->getType()->isSigned()
                    ? m_builder.CreateSDiv(left, right)
                    : m_builder.CreateUDiv(left, right)
            );
        }
        else if (node->operation == BinaryOperator::Modulo)
        {
            return (
                node->left->getType()->isSigned()
                    ? m_builder.CreateSRem(left, right)
                    : m_builder.CreateURem(left, right)
            );
        }
        else if (node->operation == BinaryOperator::BitwiseAnd)
        {
            return m_builder.CreateAnd(left, right);
        }
        else if (node->operation == BinaryOperator::BitwiseOr)
        {
            return m_builder.CreateOr(left, right);
        }
        else if (node->operation == BinaryOperator::BitwiseXor)
        {
            return m_builder.CreateXor(left, right);
        }
        else if (node->operation == BinaryOperator::ShiftLeft)
        {
            return m_builder.CreateShl(left, right);
        }
        else if (node->operation == BinaryOperator::ShiftRight)
        {
            return m_builder.CreateAShr(left, right);
        }
        else if (node->operation == BinaryOperator::LogicalAnd)
        {
            return m_builder.CreateLogicalAnd(left, right);
        }
        else if (node->operation == BinaryOperator::LogicalOr)
        {
            return m_builder.CreateLogicalOr(left, right);
        }
        else if (node->operation == BinaryOperator::Equal)
        {
            return m_builder.CreateCmp(
                llvm::CmpInst::Predicate::ICMP_EQ, left, right
            );
        }
        else if (node->operation == BinaryOperator::NotEqual)
        {
            return m_builder.CreateCmp(
                llvm::CmpInst::Predicate::ICMP_NE, left, right
            );
        }
        else if (node->operation == BinaryOperator::LessThan)
        {
            return (
                node->left->getType()->isSigned()
                    ? m_builder.CreateCmp(
                        llvm::CmpInst::Predicate::ICMP_SLT, left, right
                    )
                    : m_builder.CreateCmp(
                        llvm::CmpInst::Predicate::ICMP_ULT, left, right
                    )
            );
        }
        else if (node->operation == BinaryOperator::GreaterThan)
        {
            return (
                node->left->getType()->isSigned()
                    ? m_builder.CreateCmp(
                        llvm::CmpInst::Predicate::ICMP_SGT, left, right
                    )
                    : m_builder.CreateCmp(
                        llvm::CmpInst::Predicate::ICMP_UGT, left, right
                    )
            );
        }
        else if (node->operation == BinaryOperator::LessOrEqual)
        {
            return (
                node->left->getType()->isSigned()
                    ? m_builder.CreateCmp(
                        llvm::CmpInst::Predicate::ICMP_SLE, left, right
                    )
                    : m_builder.CreateCmp(
                        llvm::CmpInst::Predicate::ICMP_ULE, left, right
                    )
            );
        }
        else if (node->operation == BinaryOperator::GreaterOrEqual)
        {
            return (
                node->left->getType()->isSigned()
                    ? m_builder.CreateCmp(
                        llvm::CmpInst::Predicate::ICMP_SGE, left, right
                    )
                    : m_builder.CreateCmp(
                        llvm::CmpInst::Predicate::ICMP_UGE, left, right
                    )
            );
        }
        else
        {
            assert(0 && "Invalid operator in binary expression");
            return nullptr;
        }
    }
}

llvm::Value* LLVMCodegenPass::visit(IRCallExpression* node)
{
    auto target = node->getTarget();
    assert(target && "Target of call expression is undefined");

    auto function = target->getLLVMFunction();
    assert(
        function
        && "LLVM Function object for target of call expression is undefined"
    );

    std::vector<llvm::Value*> args;
    for (auto& arg : node->args)
        args.push_back(dispatch(arg));

    return m_builder.CreateCall(function, args);
}

llvm::Value* LLVMCodegenPass::visit(IRIndexExpression* node)
{
    assert(
        node->args.size() == 1
        && "Index expression must have exactly one operand"
    );
    assert(
        node->args.front()->getType()->isIntegerType()
        && "Index of index expression must be of integer type"
    );
    assert(
        (node->expression->getType()->isArrayType()
         || node->expression->getType()->isStringType())
        && "Operand of index expression must be of string or array type"
    );

    auto fieldTypes = std::vector<llvm::Type*>(
        { llvm::Type::getInt64Ty(m_llvmCtx),
          llvm::ArrayType::get(getLLVMType(node->getType()), 0) }
    );
    auto arrayType = llvm::StructType::get(m_llvmCtx, fieldTypes);

    auto indexes = std::vector<llvm::Value*>(
        { llvm::ConstantInt::get(llvm::Type::getInt64Ty(m_llvmCtx), 0),
          llvm::ConstantInt::get(llvm::Type::getInt32Ty(m_llvmCtx), 1),
          dispatch(node->args.front()) }
    );

    auto ptr =
        m_builder.CreateGEP(arrayType, dispatch(node->expression), indexes);
    return m_builder.CreateLoad(getLLVMType(node->getType()), ptr);
}

llvm::Value* LLVMCodegenPass::visit(IRFieldExpression* node)
{
    auto structType = dynamic_cast<IRStructType*>(node->expression->getType());
    for (int i = 0; i < structType->fields.size(); i++)
    {
        if (structType->fields[i].first == node->fieldName)
        {
            auto value = m_builder.CreateAlloca(
                getLLVMType(structType), nullptr, structType->name + "_"
            );
            m_builder.CreateStore(dispatch(node->expression), value);
            auto ptr =
                m_builder.CreateStructGEP(getLLVMType(structType), value, i);
            return m_builder.CreateLoad(
                getLLVMType(node->getType()),
                ptr,
                structType->toString() + "." + node->fieldName + "_"
            );
        }
    }

    assert(0 && "Unknown struct field in field expression");
    return nullptr;
}

llvm::Value* LLVMCodegenPass::visit(IRBlockStatement* node)
{
    auto prevLocalValues = m_localValues;

    for (auto& statement : node->statements)
        dispatch(statement);

    m_localValues = prevLocalValues;

    return nullptr;
}

llvm::Value* LLVMCodegenPass::visit(IRExpressionStatement* node)
{
    dispatch(node->expression);
    return nullptr;
}

llvm::Value* LLVMCodegenPass::visit(IRVariableStatement* node)
{
    for (auto& [name, value] : node->items)
    {
        assert(
            !m_localValues.contains(name) && "Local variable already defined"
        );

        m_localValues.try_emplace(
            name,
            m_builder.CreateAlloca(getLLVMType(value->getType()), nullptr, name)
        );
        m_builder.CreateStore(dispatch(value), m_localValues.at(name));
    }

    return nullptr;
}

llvm::Value* LLVMCodegenPass::visit(IRReturnStatement* node)
{
    if (node->expression)
        m_builder.CreateRet(dispatch(node->expression));
    else
        m_builder.CreateRetVoid();

    return nullptr;
}

llvm::Value* LLVMCodegenPass::visit(IRWhileStatement* node)
{
    auto parentFunction = m_builder.GetInsertBlock()->getParent();

    auto conditionBlock =
        llvm::BasicBlock::Create(m_llvmCtx, "while_cond_block");
    auto bodyBlock = llvm::BasicBlock::Create(m_llvmCtx, "while_body_block");
    auto endBlock = llvm::BasicBlock::Create(m_llvmCtx, "while_end_block");

    m_builder.CreateBr(conditionBlock);

    parentFunction->getBasicBlockList().push_back(conditionBlock);
    m_builder.SetInsertPoint(conditionBlock);
    m_builder.CreateCondBr(dispatch(node->condition), bodyBlock, endBlock);

    parentFunction->getBasicBlockList().push_back(bodyBlock);
    m_builder.SetInsertPoint(bodyBlock);
    dispatch(node->body);
    m_builder.CreateBr(conditionBlock);

    parentFunction->getBasicBlockList().push_back(endBlock);
    m_builder.SetInsertPoint(endBlock);

    return nullptr;
}

llvm::Value* LLVMCodegenPass::visit(IRIfStatement* node)
{
    auto hasElse = (node->elseBody != nullptr);
    auto parentFunction = m_builder.GetInsertBlock()->getParent();

    auto ifBlock = llvm::BasicBlock::Create(m_llvmCtx, "if_then_block");
    auto elseBlock =
        (hasElse ? llvm::BasicBlock::Create(m_llvmCtx, "if_else_block")
                 : nullptr);
    auto endBlock = llvm::BasicBlock::Create(m_llvmCtx, "if_end_block");

    m_builder.CreateCondBr(
        dispatch(node->condition), ifBlock, (hasElse ? elseBlock : endBlock)
    );

    parentFunction->getBasicBlockList().push_back(ifBlock);
    m_builder.SetInsertPoint(ifBlock);
    dispatch(node->ifBody);
    m_builder.CreateBr(endBlock);

    if (hasElse)
    {
        parentFunction->getBasicBlockList().push_back(elseBlock);
        m_builder.SetInsertPoint(elseBlock);
        dispatch(node->elseBody);
        m_builder.CreateBr(endBlock);
    }

    parentFunction->getBasicBlockList().push_back(endBlock);
    m_builder.SetInsertPoint(endBlock);

    return nullptr;
}

llvm::Value* LLVMCodegenPass::visit(IRFunctionHead* node)
{
    auto function = node->getLLVMFunction();
    assert(
        function && "No matching llvm function object for function declaration"
    );

    if (!node->body)
        return nullptr;

    auto entryBlock = llvm::BasicBlock::Create(m_llvmCtx, "entry");
    auto bodyBlock = llvm::BasicBlock::Create(m_llvmCtx, "body");

    function->getBasicBlockList().push_back(entryBlock);
    m_builder.SetInsertPoint(entryBlock);

    m_localValues.clear();
    for (int i = 0; i < node->params.size(); i++)
    {
        auto& [paramName, paramType] = node->params.at(i);
        assert(
            !m_localValues.contains(paramName)
            && "Local variable for parameter already defined"
        );

        m_localValues.try_emplace(
            paramName,
            m_builder.CreateAlloca(
                getLLVMType(paramType), nullptr, paramName + "_"
            )
        );
        m_builder.CreateStore(function->getArg(i), m_localValues.at(paramName));
    }

    m_builder.CreateBr(bodyBlock);
    function->getBasicBlockList().push_back(bodyBlock);
    m_builder.SetInsertPoint(bodyBlock);
    dispatch(node->body);

    if (node->result->isVoidType()
        && !m_builder.GetInsertBlock()->getTerminator())
        m_builder.CreateRetVoid();

    return nullptr;
}

llvm::Value* LLVMCodegenPass::visit(IRModule* node)
{
    for (auto function : node->functions)
    {
        std::vector<IRType*> params;
        for (auto& [name, type] : function->params)
            params.push_back(type);

        std::vector<llvm::Type*> llvmParams;
        for (auto& param : params)
            llvmParams.push_back(getLLVMType(param));

        auto type = llvm::FunctionType::get(
            getLLVMType(function->result), llvmParams, false
        );
        auto name =
            ((function->body) ? getMangledFunctionName(function->name, params)
                              : function->name);
        auto llvmFunction = llvm::Function::Create(
            type,
            llvm::GlobalValue::LinkageTypes::ExternalLinkage,
            name,
            m_llvmModule
        );
        function->setLLVMFunction(llvmFunction);

        if (!function->body)
        {
            llvmFunction->setCallingConv(llvm::CallingConv::Win64);
            llvmFunction->setDLLStorageClass(
                llvm::GlobalValue::DLLStorageClassTypes::DLLImportStorageClass
            );
        }
    }

    for (auto function : node->functions)
        dispatch(function);

    return nullptr;
}

llvm::Type* LLVMCodegenPass::getLLVMType(IRType* type)
{
    if (m_llvmTypes.contains(type))
    {
        return m_llvmTypes.at(type);
    }
    else if (type->isVoidType())
    {
        m_llvmTypes.try_emplace(type, llvm::Type::getVoidTy(m_llvmCtx));
        return m_llvmTypes.at(type);
    }
    else if (type->isBoolType())
    {
        m_llvmTypes.try_emplace(type, llvm::Type::getInt1Ty(m_llvmCtx));
        return m_llvmTypes.at(type);
    }
    else if (type->isIntegerType())
    {
        m_llvmTypes.try_emplace(
            type,
            llvm::Type::getIntNTy(m_llvmCtx, (unsigned int)type->getBitSize())
        );
        return m_llvmTypes.at(type);
    }
    else if (type->isCharType())
    {
        m_llvmTypes.try_emplace(type, llvm::Type::getInt32Ty(m_llvmCtx));
        return m_llvmTypes.at(type);
    }
    else if (type->isStringType())
    {
        m_llvmTypes.try_emplace(
            type, getLLVMType(m_compCtx.getArrayType(m_compCtx.getU8()))
        );
        return m_llvmTypes.at(type);
    }
    else if (type->isStructType())
    {
        auto structType = (IRStructType*)type;

        std::vector<llvm::Type*> fields;
        for (auto& [fieldName, fieldType] : structType->fields)
            fields.push_back(getLLVMType(fieldType));

        m_llvmTypes.try_emplace(type, llvm::StructType::get(m_llvmCtx, fields));
        return m_llvmTypes.at(type);
    }
    else if (type->isPointerType())
    {
        auto base = ((IRPointerType*)type)->base;
        m_llvmTypes.try_emplace(type, getLLVMType(base)->getPointerTo());
        return m_llvmTypes.at(type);
    }
    else if (type->isArrayType())
    {
        auto base = ((IRArrayType*)type)->base;
        auto fields = std::vector<llvm::Type*>(
            { llvm::Type::getInt64Ty(m_llvmCtx),
              llvm::ArrayType::get(getLLVMType(base), 0) }
        );

        m_llvmTypes.try_emplace(
            type, llvm::StructType::get(m_llvmCtx, fields)->getPointerTo()
        );
        return m_llvmTypes.at(type);
    }
    else
    {
        throw std::exception();
    }
}

std::string LLVMCodegenPass::getMangledTypeName(IRType* type)
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
        return (type->isSigned() ? "I" : "U")
            + std::to_string(type->getBitSize());
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
        auto structType = (IRStructType*)type;
        auto output = "S_" + structType->name + "_";

        for (auto& [fieldName, fieldType] : structType->fields)
            output += getMangledTypeName(fieldType);

        output += "_";
        return output;
    }
    else if (type->isPointerType())
    {
        auto ptrType = (IRPointerType*)type;
        return "P_" + getMangledTypeName(ptrType->base) + "_";
    }
    else if (type->isArrayType())
    {
        auto arrType = (IRArrayType*)type;
        return "A_" + getMangledTypeName(arrType->base) + "_";
    }
    else
    {
        throw std::exception();
    }
}

std::string LLVMCodegenPass::getMangledFunctionName(
    std::string const& function, std::vector<IRType*> const& params
)
{
    auto output = function + "@";
    for (auto& param : params)
        output += getMangledTypeName(param);

    return output;
}