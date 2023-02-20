#pragma once
#include <ostream>
#include <string>
#include <string_view>

#include "../compiler.hpp"
#include "../data/operator.hpp"
#include "../ir/ir.hpp"
#include "../util/error_logger.hpp"

class OperatorLoweringPass : protected IRVisitor<IRNode*>
{
private:
    ErrorLogger& m_logger;
    CompilationContext& m_compCtx;
    ModuleContext& m_modCtx;
    GraphContext& m_irCtx;

public:
    OperatorLoweringPass(
        ErrorLogger& logger,
        CompilationContext& compCtx,
        ModuleContext& modCtx,
        GraphContext& irCtx
    )
        : m_logger(logger), m_compCtx(compCtx), m_modCtx(modCtx), m_irCtx(irCtx)
    {
    }

public:
    IRSourceFile* process(IRSourceFile* source);

protected:
    virtual IRNode* visit(IRIntegerExpression* node) override;
    virtual IRNode* visit(IRBoolExpression* node) override;
    virtual IRNode* visit(IRCharExpression* node) override;
    virtual IRNode* visit(IRStringExpression* node) override;
    virtual IRNode* visit(IRIdentifierExpression* node) override;
    virtual IRNode* visit(IRStructExpression* node) override;
    virtual IRNode* visit(IRUnaryExpression* node) override;
    virtual IRNode* visit(IRBinaryExpression* node) override;
    virtual IRNode* visit(IRCallExpression* node) override;
    virtual IRNode* visit(IRIndexExpression* node) override;
    virtual IRNode* visit(IRFieldExpression* node) override;

    virtual IRNode* visit(IRBlockStatement* node) override;
    virtual IRNode* visit(IRExpressionStatement* node) override;
    virtual IRNode* visit(IRVariableStatement* node) override;
    virtual IRNode* visit(IRReturnStatement* node) override;
    virtual IRNode* visit(IRWhileStatement* node) override;
    virtual IRNode* visit(IRIfStatement* node) override;

    virtual IRNode* visit(IRConstraintDeclaration* node) override;
    virtual IRNode* visit(IRStructDeclaration* node) override;
    virtual IRNode* visit(IRFunctionDeclaration* node) override;
    virtual IRNode* visit(IRSourceFile* node) override;
};