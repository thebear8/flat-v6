#include "codegen_pass.hpp"

#include <llvm/Analysis/CGSCCPassManager.h>
#include <llvm/Analysis/LoopAnalysisManager.h>
#include <llvm/Passes/PassBuilder.h>

void LLVMCodegenPass::process(IRSourceFile* source) {
    for (auto& [functionName, function] : modCtx.getFunctionList()) {
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
            type, llvm::GlobalValue::LinkageTypes::ExternalLinkage, name, mod
        );
        compCtx.addLLVMFunction(function, llvmFunction);

        if (!function->body) {
            llvmFunction->setCallingConv(llvm::CallingConv::Win64);
            llvmFunction->setDLLStorageClass(
                llvm::GlobalValue::DLLStorageClassTypes::DLLImportStorageClass
            );
        }
    }

    dispatch(source);
}

void LLVMCodegenPass::optimize() {
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

llvm::Value* LLVMCodegenPass::visit(IRIntegerExpression* node) {
    auto type = llvm::IntegerType::get(llvmCtx, (unsigned int)node->width);
    return llvm::ConstantInt::get(type, node->value, (uint8_t)node->radix);
}

llvm::Value* LLVMCodegenPass::visit(IRBoolExpression* node) {
    return llvm::ConstantInt::get(llvm::Type::getInt1Ty(llvmCtx), node->value);
}

llvm::Value* LLVMCodegenPass::visit(IRCharExpression* node) {
    return llvm::ConstantInt::get(llvm::Type::getInt32Ty(llvmCtx), node->value);
}

llvm::Value* LLVMCodegenPass::visit(IRStringExpression* node) {
    std::vector<llvm::Constant*> stringBytes;
    for (auto c : node->value)
        stringBytes.push_back(
            llvm::ConstantInt::get(llvm::Type::getInt8Ty(llvmCtx), c)
        );

    auto fieldTypes = std::vector<llvm::Type*>(
        { llvm::Type::getInt64Ty(llvmCtx),
          llvm::ArrayType::get(
              llvm::Type::getInt8Ty(llvmCtx), stringBytes.size()
          ) }
    );
    auto structType = llvm::StructType::get(llvmCtx, fieldTypes);

    auto fieldValues = std::vector<llvm::Constant*>(
        { llvm::ConstantInt::get(
              llvm::Type::getInt64Ty(llvmCtx), stringBytes.size()
          ),
          llvm::ConstantArray::get(
              llvm::ArrayType::get(
                  llvm::Type::getInt8Ty(llvmCtx), stringBytes.size()
              ),
              stringBytes
          ) }
    );
    auto structValue = llvm::ConstantStruct::get(structType, fieldValues);

    return new llvm::GlobalVariable(
        mod,
        structType,
        true,
        llvm::GlobalValue::LinkageTypes::InternalLinkage,
        structValue
    );
}

llvm::Value* LLVMCodegenPass::visit(IRIdentifierExpression* node) {
    assert(
        localValues.contains(node->value)
        && "Undefined local Variable in identifier expression"
    );
    return builder.CreateLoad(
        getLLVMType(node->type), localValues.at(node->value), node->value + "_"
    );
}

llvm::Value* LLVMCodegenPass::visit(IRStructExpression* node) {
    auto type = dynamic_cast<IRStructType*>(node->type);
    auto structPtr =
        builder.CreateAlloca(getLLVMType(type), nullptr, type->name + "_");

    for (int i = 0; i < type->fields.size(); i++) {
        for (int j = 0; j < node->fields.size(); j++) {
            if (node->fields.at(j).first == type->fields.at(i).first) {
                auto const& fieldName = node->fields.at(j).first;
                auto fieldValue = dispatch(node->fields.at(j).second);
                auto fieldPtr = builder.CreateStructGEP(
                    getLLVMType(type),
                    structPtr,
                    i,
                    type->name + "." + fieldName + "_"
                );
                builder.CreateStore(fieldValue, fieldPtr);
                break;
            }

            if (j == node->fields.size())
                assert(0 && "No initializer for field in struct expression");
        }
    }

    return builder.CreateLoad(getLLVMType(type), structPtr);
}

llvm::Value* LLVMCodegenPass::visit(IRUnaryExpression* node) {
    if (node->operation == UnaryOperator::Positive) {
        return dispatch(node->expression);
    } else if (node->operation == UnaryOperator::Negative) {
        return builder.CreateNeg(dispatch(node->expression));
    } else if (node->operation == UnaryOperator::BitwiseNot) {
        return builder.CreateNot(dispatch(node->expression));
    } else if (node->operation == UnaryOperator::LogicalNot) {
        return builder.CreateNot(dispatch(node->expression));
    } else {
        assert(0 && "Invalid operator in unary expression");
        return nullptr;
    }
}

llvm::Value* LLVMCodegenPass::visit(IRBinaryExpression* node) {
    if (dynamic_cast<IRIdentifierExpression*>(node->left)) {
        auto const& name =
            dynamic_cast<IRIdentifierExpression*>(node->left)->value;
        assert(localValues.contains(name) && "Undefined local variable");

        builder.CreateStore(dispatch(node->right), localValues.at(name));
        return builder.CreateLoad(
            getLLVMType(node->type), localValues.at(name), name + "_"
        );
    } else {
        auto left = dispatch(node->left);
        auto right =
            ((node->left->type->isSigned())
                 ? builder.CreateSExtOrTrunc(
                     dispatch(node->right), getLLVMType(node->left->type)
                 )
                 : builder.CreateZExtOrTrunc(
                     dispatch(node->right), getLLVMType(node->left->type)
                 ));

        if (node->operation == BinaryOperator::Add) {
            return builder.CreateAdd(left, right);
        } else if (node->operation == BinaryOperator::Subtract) {
            return builder.CreateSub(left, right);
        } else if (node->operation == BinaryOperator::Multiply) {
            return builder.CreateMul(left, right);
        } else if (node->operation == BinaryOperator::Divide) {
            return (
                node->left->type->isSigned() ? builder.CreateSDiv(left, right)
                                             : builder.CreateUDiv(left, right)
            );
        } else if (node->operation == BinaryOperator::Modulo) {
            return (
                node->left->type->isSigned() ? builder.CreateSRem(left, right)
                                             : builder.CreateURem(left, right)
            );
        } else if (node->operation == BinaryOperator::BitwiseAnd) {
            return builder.CreateAnd(left, right);
        } else if (node->operation == BinaryOperator::BitwiseOr) {
            return builder.CreateOr(left, right);
        } else if (node->operation == BinaryOperator::BitwiseXor) {
            return builder.CreateXor(left, right);
        } else if (node->operation == BinaryOperator::ShiftLeft) {
            return builder.CreateShl(left, right);
        } else if (node->operation == BinaryOperator::ShiftRight) {
            return builder.CreateAShr(left, right);
        } else if (node->operation == BinaryOperator::LogicalAnd) {
            return builder.CreateLogicalAnd(left, right);
        } else if (node->operation == BinaryOperator::LogicalOr) {
            return builder.CreateLogicalOr(left, right);
        } else if (node->operation == BinaryOperator::Equal) {
            return builder.CreateCmp(
                llvm::CmpInst::Predicate::ICMP_EQ, left, right
            );
        } else if (node->operation == BinaryOperator::NotEqual) {
            return builder.CreateCmp(
                llvm::CmpInst::Predicate::ICMP_NE, left, right
            );
        } else if (node->operation == BinaryOperator::LessThan) {
            return (
                node->left->type->isSigned()
                    ? builder.CreateCmp(
                        llvm::CmpInst::Predicate::ICMP_SLT, left, right
                    )
                    : builder.CreateCmp(
                        llvm::CmpInst::Predicate::ICMP_ULT, left, right
                    )
            );
        } else if (node->operation == BinaryOperator::GreaterThan) {
            return (
                node->left->type->isSigned()
                    ? builder.CreateCmp(
                        llvm::CmpInst::Predicate::ICMP_SGT, left, right
                    )
                    : builder.CreateCmp(
                        llvm::CmpInst::Predicate::ICMP_UGT, left, right
                    )
            );
        } else if (node->operation == BinaryOperator::LessOrEqual) {
            return (
                node->left->type->isSigned()
                    ? builder.CreateCmp(
                        llvm::CmpInst::Predicate::ICMP_SLE, left, right
                    )
                    : builder.CreateCmp(
                        llvm::CmpInst::Predicate::ICMP_ULE, left, right
                    )
            );
        } else if (node->operation == BinaryOperator::GreaterOrEqual) {
            return (
                node->left->type->isSigned()
                    ? builder.CreateCmp(
                        llvm::CmpInst::Predicate::ICMP_SGE, left, right
                    )
                    : builder.CreateCmp(
                        llvm::CmpInst::Predicate::ICMP_UGE, left, right
                    )
            );
        } else {
            assert(0 && "Invalid operator in binary expression");
            return nullptr;
        }
    }
}

llvm::Value* LLVMCodegenPass::visit(IRCallExpression* node) {
    auto target = node->target;
    assert(target && "Target of call expression is undefined");

    auto function = compCtx.getLLVMFunction(target);
    assert(
        function
        && "LLVM Function object for target of call expression is undefined"
    );

    std::vector<llvm::Value*> args;
    for (auto& arg : node->args)
        args.push_back(dispatch(arg));

    return builder.CreateCall(function, args);
}

llvm::Value* LLVMCodegenPass::visit(IRIndexExpression* node) {
    assert(
        node->args.size() == 1
        && "Index expression must have exactly one operand"
    );
    assert(
        node->args.front()->type->isIntegerType()
        && "Index of index expression must be of integer type"
    );
    assert(
        (node->expression->type->isArrayType()
         || node->expression->type->isStringType())
        && "Operand of index expression must be of string or array type"
    );

    auto fieldTypes = std::vector<llvm::Type*>(
        { llvm::Type::getInt64Ty(llvmCtx),
          llvm::ArrayType::get(getLLVMType(node->type), 0) }
    );
    auto arrayType = llvm::StructType::get(llvmCtx, fieldTypes);

    auto indexes = std::vector<llvm::Value*>(
        { llvm::ConstantInt::get(llvm::Type::getInt64Ty(llvmCtx), 0),
          llvm::ConstantInt::get(llvm::Type::getInt32Ty(llvmCtx), 1),
          dispatch(node->args.front()) }
    );

    auto ptr =
        builder.CreateGEP(arrayType, dispatch(node->expression), indexes);
    return builder.CreateLoad(getLLVMType(node->type), ptr);
}

llvm::Value* LLVMCodegenPass::visit(IRFieldExpression* node) {
    auto structType = dynamic_cast<IRStructType*>(node->expression->type);
    for (int i = 0; i < structType->fields.size(); i++) {
        if (structType->fields[i].first == node->fieldName) {
            auto value = builder.CreateAlloca(
                getLLVMType(structType), nullptr, structType->name + "_"
            );
            builder.CreateStore(dispatch(node->expression), value);
            auto ptr =
                builder.CreateStructGEP(getLLVMType(structType), value, i);
            return builder.CreateLoad(
                getLLVMType(node->type),
                ptr,
                structType->toString() + "." + node->fieldName + "_"
            );
        }
    }

    assert(0 && "Unknown struct field in field expression");
    return nullptr;
}

llvm::Value* LLVMCodegenPass::visit(IRBlockStatement* node) {
    auto prevLocalValues = localValues;

    for (auto& statement : node->statements)
        dispatch(statement);

    localValues = prevLocalValues;

    return nullptr;
}

llvm::Value* LLVMCodegenPass::visit(IRExpressionStatement* node) {
    dispatch(node->expression);
    return nullptr;
}

llvm::Value* LLVMCodegenPass::visit(IRVariableStatement* node) {
    for (auto& [name, value] : node->items) {
        assert(!localValues.contains(name) && "Local variable already defined");

        localValues.try_emplace(
            name, builder.CreateAlloca(getLLVMType(value->type), nullptr, name)
        );
        builder.CreateStore(dispatch(value), localValues.at(name));
    }

    return nullptr;
}

llvm::Value* LLVMCodegenPass::visit(IRReturnStatement* node) {
    if (node->expression)
        builder.CreateRet(dispatch(node->expression));
    else
        builder.CreateRetVoid();

    return nullptr;
}

llvm::Value* LLVMCodegenPass::visit(IRWhileStatement* node) {
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

llvm::Value* LLVMCodegenPass::visit(IRIfStatement* node) {
    auto hasElse = (node->elseBody != nullptr);
    auto parentFunction = builder.GetInsertBlock()->getParent();

    auto ifBlock = llvm::BasicBlock::Create(llvmCtx, "if_then_block");
    auto elseBlock =
        (hasElse ? llvm::BasicBlock::Create(llvmCtx, "if_else_block") : nullptr
        );
    auto endBlock = llvm::BasicBlock::Create(llvmCtx, "if_end_block");

    builder.CreateCondBr(
        dispatch(node->condition), ifBlock, (hasElse ? elseBlock : endBlock)
    );

    parentFunction->getBasicBlockList().push_back(ifBlock);
    builder.SetInsertPoint(ifBlock);
    dispatch(node->ifBody);
    builder.CreateBr(endBlock);

    if (hasElse) {
        parentFunction->getBasicBlockList().push_back(elseBlock);
        builder.SetInsertPoint(elseBlock);
        dispatch(node->elseBody);
        builder.CreateBr(endBlock);
    }

    parentFunction->getBasicBlockList().push_back(endBlock);
    builder.SetInsertPoint(endBlock);

    return nullptr;
}

llvm::Value* LLVMCodegenPass::visit(IRStructDeclaration* node) {
    return nullptr;
}

llvm::Value* LLVMCodegenPass::visit(IRFunctionDeclaration* node) {
    auto function = compCtx.getLLVMFunction(node);
    assert(
        function && "No matching llvm function object for function declaration"
    );

    if (!node->body)
        return nullptr;

    auto entryBlock = llvm::BasicBlock::Create(llvmCtx, "entry");
    auto bodyBlock = llvm::BasicBlock::Create(llvmCtx, "body");

    function->getBasicBlockList().push_back(entryBlock);
    builder.SetInsertPoint(entryBlock);

    localValues.clear();
    for (int i = 0; i < node->params.size(); i++) {
        auto& [paramName, paramType] = node->params.at(i);
        assert(
            !localValues.contains(paramName)
            && "Local variable for parameter already defined"
        );

        localValues.try_emplace(
            paramName,
            builder.CreateAlloca(
                getLLVMType(paramType), nullptr, paramName + "_"
            )
        );
        builder.CreateStore(function->getArg(i), localValues.at(paramName));
    }

    builder.CreateBr(bodyBlock);
    function->getBasicBlockList().push_back(bodyBlock);
    builder.SetInsertPoint(bodyBlock);
    dispatch(node->body);

    if (node->result->isVoidType()
        && !builder.GetInsertBlock()->getTerminator())
        builder.CreateRetVoid();

    return nullptr;
}

llvm::Value* LLVMCodegenPass::visit(IRSourceFile* node) {
    for (auto& decl : node->declarations)
        dispatch(decl);

    return nullptr;
}

llvm::Type* LLVMCodegenPass::getLLVMType(IRType* type) {
    if (llvmTypes.contains(type)) {
        return llvmTypes.at(type);
    } else if (type->isVoidType()) {
        llvmTypes.try_emplace(type, llvm::Type::getVoidTy(llvmCtx));
        return llvmTypes.at(type);
    } else if (type->isBoolType()) {
        llvmTypes.try_emplace(type, llvm::Type::getInt1Ty(llvmCtx));
        return llvmTypes.at(type);
    } else if (type->isIntegerType()) {
        llvmTypes.try_emplace(
            type,
            llvm::Type::getIntNTy(llvmCtx, (unsigned int)type->getBitSize())
        );
        return llvmTypes.at(type);
    } else if (type->isCharType()) {
        llvmTypes.try_emplace(type, llvm::Type::getInt32Ty(llvmCtx));
        return llvmTypes.at(type);
    } else if (type->isStringType()) {
        llvmTypes.try_emplace(
            type, getLLVMType(compCtx.getArrayType(compCtx.getU8()))
        );
        return llvmTypes.at(type);
    } else if (type->isStructType()) {
        auto structType = dynamic_cast<IRStructType*>(type);

        std::vector<llvm::Type*> fields;
        for (auto& [fieldName, fieldType] : structType->fields)
            fields.push_back(getLLVMType(fieldType));

        llvmTypes.try_emplace(type, llvm::StructType::get(llvmCtx, fields));
        return llvmTypes.at(type);
    } else if (type->isPointerType()) {
        auto base = dynamic_cast<IRPointerType*>(type)->base;
        llvmTypes.try_emplace(type, getLLVMType(base)->getPointerTo());
        return llvmTypes.at(type);
    } else if (type->isArrayType()) {
        auto base = dynamic_cast<IRArrayType*>(type)->base;
        auto fields = std::vector<llvm::Type*>(
            { llvm::Type::getInt64Ty(llvmCtx),
              llvm::ArrayType::get(getLLVMType(base), 0) }
        );

        llvmTypes.try_emplace(
            type, llvm::StructType::get(llvmCtx, fields)->getPointerTo()
        );
        return llvmTypes.at(type);
    } else {
        throw std::exception();
    }
}

std::string LLVMCodegenPass::getMangledTypeName(IRType* type) {
    if (type->isVoidType()) {
        return "V";
    } else if (type->isBoolType()) {
        return "B";
    } else if (type->isIntegerType()) {
        return (type->isSigned() ? "I" : "U")
            + std::to_string(type->getBitSize());
    } else if (type->isCharType()) {
        return "C";
    } else if (type->isStringType()) {
        return "Str";
    } else if (type->isStructType()) {
        auto structType = dynamic_cast<IRStructType*>(type);
        auto output = "S_" + structType->name + "_";

        for (auto& [fieldName, fieldType] : structType->fields)
            output += getMangledTypeName(fieldType);

        output += "_";
        return output;
    } else if (type->isPointerType()) {
        auto ptrType = dynamic_cast<IRPointerType*>(type);
        return "P_" + getMangledTypeName(ptrType->base) + "_";
    } else if (type->isArrayType()) {
        auto arrType = dynamic_cast<IRArrayType*>(type);
        return "A_" + getMangledTypeName(arrType->base) + "_";
    } else {
        throw std::exception();
    }
}

std::string LLVMCodegenPass::getMangledFunctionName(
    std::string const& function, std::vector<IRType*> const& params
) {
    auto output = function + "@";
    for (auto& param : params)
        output += getMangledTypeName(param);

    return output;
}