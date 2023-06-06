#pragma once
#include <ostream>
#include <string>
#include <string_view>

#include "../../ir/ir.hpp"

class ErrorLogger;
class CompilationContext;
class GraphContext;

class OperatorLoweringPass : protected IRVisitor<void>
{
private:
    ErrorLogger& m_logger;
    CompilationContext& m_compCtx;

    IRModule* m_module = nullptr;
    GraphContext* m_irCtx = nullptr;

public:
    OperatorLoweringPass(ErrorLogger& logger, CompilationContext& compCtx)
        : m_logger(logger), m_compCtx(compCtx)
    {
    }

public:
    void process(IRModule* node);

private:
    virtual void visit(IRIntegerExpression* node) override {}
    virtual void visit(IRBoolExpression* node) override {}
    virtual void visit(IRCharExpression* node) override {}
    virtual void visit(IRStringExpression* node) override {}
    virtual void visit(IRIdentifierExpression* node) override {}
    virtual void visit(IRStructExpression* node) override;
    virtual void visit(IRUnaryExpression* node, IRNode*& ref) override;
    virtual void visit(IRBinaryExpression* node, IRNode*& ref) override;
    virtual void visit(IRCallExpression* node, IRNode*& ref) override;
    virtual void visit(IRIndexExpression* node, IRNode*& ref) override;
    virtual void visit(IRFieldExpression* node) override;

    virtual void visit(IRBlockStatement* node) override;
    virtual void visit(IRExpressionStatement* node) override;
    virtual void visit(IRVariableStatement* node) override;
    virtual void visit(IRReturnStatement* node) override;
    virtual void visit(IRWhileStatement* node) override;
    virtual void visit(IRIfStatement* node) override;

    virtual void visit(IRNormalFunction* node) override;
};