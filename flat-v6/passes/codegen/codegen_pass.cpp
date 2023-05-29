#include "codegen_pass.hpp"

#include <llvm/Analysis/CGSCCPassManager.h>
#include <llvm/Analysis/LoopAnalysisManager.h>
#include <llvm/Passes/PassBuilder.h>

#include "../../util/assert.hpp"

void LLVMCodegenPass::process(IRModule* node)
{
    for (auto [functionTemplate, function] :
         node->getEnv()->getFunctionInstantiationMap())
    {
        if (!function->isNormalFunction())
            continue;

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

    for (auto [functionTemplate, function] :
         node->getEnv()->getFunctionInstantiationMap())
    {
        if (!(function->isNormalFunction() && function->body))
            continue;

        generateFunctionBody((IRNormalFunction*)function);
    }
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
    FLC_ASSERT(m_env);
    FLC_ASSERT(
        m_env->findVariableValue(node->value),
        "Undefined local Variable in identifier expression"
    );
    return m_builder.CreateLoad(
        getLLVMType(node->getType()),
        m_env->findVariableValue(node->value),
        node->value + "_"
    );
}

llvm::Value* LLVMCodegenPass::visit(IRStructExpression* node)
{
    auto type = dynamic_cast<IRStruct*>(node->getType());
    auto structPtr =
        m_builder.CreateAlloca(getLLVMType(type), nullptr, type->name + "_");

    size_t idx = 0;
    for (auto [fieldName, fieldType] : type->fields)
    {
        if (!node->fields.contains(fieldName))
        {
            FLC_ASSERT(0, "No initializer for field in struct expression");
            throw std::exception();
        }

        auto fieldValue = dispatch(node->fields.at(fieldName));
        auto fieldPtr = m_builder.CreateStructGEP(
            getLLVMType(type),
            structPtr,
            idx++,
            type->name + "." + fieldName + "_"
        );
        m_builder.CreateStore(fieldValue, fieldPtr);
        break;
    }

    return m_builder.CreateLoad(getLLVMType(type), structPtr);
}

llvm::Value* LLVMCodegenPass::visit(IRBoundCallExpression* node)
{
    FLC_ASSERT(node->target);

    m_args.push({});
    for (auto arg : node->args)
        m_args.top().push_back(arg);

    auto call = dispatch(node->target);
    m_args.pop();
    return call;
}

llvm::Value* LLVMCodegenPass::visit(IRNormalFunction* node)
{
    FLC_ASSERT(!m_args.empty());
    FLC_ASSERT(node->getLLVMFunction(nullptr));

    std::vector<llvm::Value*> args;
    for (auto arg : m_args.top())
        args.push_back(dispatch(arg));

    return m_builder.CreateCall(node->getLLVMFunction(), args);
}

llvm::Value* LLVMCodegenPass::visit(IRUnaryIntrinsic* node)
{
    FLC_ASSERT(!m_args.empty());
    FLC_ASSERT(m_args.top().size() == 1);

    auto value = dispatch(m_args.top().front());

    if (node->name == "__pos__")
        return value;
    else if (node->name == "__neg__")
        return m_builder.CreateNeg(value);
    else if (node->name == "__not__")
        return m_builder.CreateNot(value);
    else if (node->name == "__lnot__")
        return m_builder.CreateNot(value);

    FLC_ASSERT(false);
    return nullptr;
}

llvm::Value* LLVMCodegenPass::visit(IRBinaryIntrinsic* node)
{
    FLC_ASSERT(!m_args.empty());
    FLC_ASSERT(m_args.top().size() == 2);

    auto left = dispatch(m_args.top().front());
    auto right = (m_args.top().back()->getType()->isSigned())
        ? m_builder.CreateSExtOrTrunc(
            dispatch(m_args.top().back()),
            getLLVMType(m_args.top().back()->getType())
        )
        : m_builder.CreateZExtOrTrunc(
            dispatch(m_args.top().back()),
            getLLVMType(m_args.top().back()->getType())
        );

    if (node->name == "__add__")
        return m_builder.CreateAdd(left, right);
    else if (node->name == "__subtract__")
        return m_builder.CreateSub(left, right);
    else if (node->name == "__mul__")
        return m_builder.CreateMul(left, right);
    else if (node->name == "__div__")
        return (m_args.top().front()->getType()->isSigned())
            ? m_builder.CreateSDiv(left, right)
            : m_builder.CreateUDiv(left, right);
    else if (node->name == "__mod__")
        return (m_args.top().front()->getType()->isSigned())
            ? m_builder.CreateSRem(left, right)
            : m_builder.CreateURem(left, right);

    if (node->name == "__and__")
        return m_builder.CreateAnd(left, right);
    else if (node->name == "__or__")
        return m_builder.CreateOr(left, right);
    else if (node->name == "__xor__")
        return m_builder.CreateXor(left, right);
    else if (node->name == "__shl__")
        return m_builder.CreateShl(left, right);
    else if (node->name == "__shr__")
        return m_builder.CreateAShr(left, right);

    if (node->name == "__land__")
        return m_builder.CreateLogicalAnd(left, right);
    else if (node->name == "__lor__")
        return m_builder.CreateLogicalOr(left, right);

    if (node->name == "__eq__")
        return m_builder.CreateCmp(
            llvm::CmpInst::Predicate::ICMP_EQ, left, right
        );
    else if (node->name == "__ne__")
        return m_builder.CreateCmp(
            llvm::CmpInst::Predicate::ICMP_NE, left, right
        );

    if (node->name == "__lt__")
        return (m_args.top().front()->getType()->isSigned())
            ? m_builder.CreateCmp(
                llvm::CmpInst::Predicate::ICMP_SLT, left, right
            )
            : m_builder.CreateCmp(
                llvm::CmpInst::Predicate::ICMP_ULT, left, right
            );
    else if (node->name == "__gt__")
        return (m_args.top().front()->getType()->isSigned())
            ? m_builder.CreateCmp(
                llvm::CmpInst::Predicate::ICMP_SGT, left, right
            )
            : m_builder.CreateCmp(
                llvm::CmpInst::Predicate::ICMP_SLT, left, right
            );
    else if (node->name == "__lteq__")
        return (m_args.top().front()->getType()->isSigned())
            ? m_builder.CreateCmp(
                llvm::CmpInst::Predicate::ICMP_SLE, left, right
            )
            : m_builder.CreateCmp(
                llvm::CmpInst::Predicate::ICMP_ULE, left, right
            );
    else if (node->name == "__gteq__")
        return (m_args.top().front()->getType()->isSigned())
            ? m_builder.CreateCmp(
                llvm::CmpInst::Predicate::ICMP_SGE, left, right
            )
            : m_builder.CreateCmp(
                llvm::CmpInst::Predicate::ICMP_SLE, left, right
            );

    FLC_ASSERT(false);
    return nullptr;
}

llvm::Value* LLVMCodegenPass::visit(IRIndexIntrinsic* node)
{
    FLC_ASSERT(!m_args.empty());
    FLC_ASSERT(m_args.size() == 2);
    FLC_ASSERT(
        m_args.top().front()->getType()->isArrayType()
        || m_args.top().front()->getType()->isStringType()
    );
    FLC_ASSERT(m_args.top().back()->getType()->isIntegerType());

    auto array = dispatch(m_args.top().front());
    auto index = dispatch(m_args.top().back());

    auto fieldTypes = std::vector<llvm::Type*>(
        { llvm::Type::getInt64Ty(m_llvmCtx),
          llvm::ArrayType::get(getLLVMType(node->result), 0) }
    );
    auto arrayType = llvm::StructType::get(m_llvmCtx, fieldTypes);

    auto ptr = m_builder.CreateGEP(
        arrayType,
        array,
        { llvm::ConstantInt::get(llvm::Type::getInt64Ty(m_llvmCtx), 0),
          llvm::ConstantInt::get(llvm::Type::getInt32Ty(m_llvmCtx), 1),
          index }
    );

    return m_builder.CreateLoad(getLLVMType(node->result), ptr);
}

llvm::Value* LLVMCodegenPass::visit(IRFieldExpression* node)
{
    auto structType = dynamic_cast<IRStruct*>(node->expression->getType());

    FLC_ASSERT(
        structType->fields.contains(node->fieldName),
        "Unknown struct field in field expression"
    );

    auto value = m_builder.CreateAlloca(
        getLLVMType(structType), nullptr, structType->name + "_"
    );
    m_builder.CreateStore(dispatch(node->expression), value);

    auto idx = std::distance(
        structType->fields.begin(), structType->fields.find(node->fieldName)
    );
    auto ptr = m_builder.CreateStructGEP(getLLVMType(structType), value, idx);

    return m_builder.CreateLoad(
        getLLVMType(node->getType()),
        ptr,
        structType->toString() + "." + node->fieldName + "_"
    );
}

llvm::Value* LLVMCodegenPass::visit(IRBlockStatement* node)
{
    m_env = m_envCtx.make(
        Environment("BlockStatement@" + std::to_string((size_t)node), m_env)
    );

    for (auto& statement : node->statements)
        dispatch(statement);

    m_env = m_env->getParent();
    return nullptr;
}

llvm::Value* LLVMCodegenPass::visit(IRExpressionStatement* node)
{
    dispatch(node->expression);
    return nullptr;
}

llvm::Value* LLVMCodegenPass::visit(IRVariableStatement* node)
{
    FLC_ASSERT(m_env);
    for (auto& [name, value] : node->items)
    {
        FLC_ASSERT(
            m_env->findVariableValue(name), "Local variable already defined"
        );

        m_env->setVariableValue(
            name,
            m_builder.CreateAlloca(getLLVMType(value->getType()), nullptr, name)
        );
        m_builder.CreateStore(dispatch(value), m_env->findVariableValue(name));
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

void LLVMCodegenPass::generateFunctionBody(IRNormalFunction* node)
{
    auto function = node->getLLVMFunction();
    FLC_ASSERT(
        function, "No matching llvm function object for function declaration"
    );

    if (!node->body)
        return;

    auto entryBlock = llvm::BasicBlock::Create(m_llvmCtx, "entry");
    auto bodyBlock = llvm::BasicBlock::Create(m_llvmCtx, "body");

    function->getBasicBlockList().push_back(entryBlock);
    m_builder.SetInsertPoint(entryBlock);

    m_env = m_envCtx.make(Environment(node->name, m_env));
    for (int i = 0; i < node->params.size(); i++)
    {
        auto& [paramName, paramType] = node->params.at(i);
        FLC_ASSERT(
            !m_env->findVariableValue(paramName),
            "Local variable for parameter already defined"
        );

        m_env->setVariableValue(
            paramName,
            m_builder.CreateAlloca(
                getLLVMType(paramType), nullptr, paramName + "_"
            )
        );
        m_builder.CreateStore(
            function->getArg(i), m_env->findVariableValue(paramName)
        );
    }

    m_builder.CreateBr(bodyBlock);
    function->getBasicBlockList().push_back(bodyBlock);
    m_builder.SetInsertPoint(bodyBlock);
    dispatch(node->body);

    if (node->result->isVoidType()
        && !m_builder.GetInsertBlock()->getTerminator())
        m_builder.CreateRetVoid();

    m_env = m_env->getParent();
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
        auto structType = (IRStruct*)type;

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
        auto structType = (IRStruct*)type;
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

    FLC_ASSERT(false);
    return nullptr;
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