#pragma once
#include <ostream>
#include <string>
#include <string_view>

#include "../compiler.hpp"
#include "../data/ir.hpp"
#include "../data/operator.hpp"
#include "../util/error_logger.hpp"

class OperatorLoweringPass : protected IRVisitor<IRNode*>
{
private:
    ErrorLogger& logger;
    CompilationContext& compCtx;
    ModuleContext& modCtx;
    GraphContext& irCtx;

public:
    OperatorLoweringPass(
        ErrorLogger& logger,
        CompilationContext& compCtx,
        ModuleContext& modCtx,
        GraphContext& irCtx)
        : logger(logger), compCtx(compCtx), modCtx(modCtx), irCtx(irCtx)
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

    virtual IRNode* visit(IRStructDefinition* node) override;
    virtual IRNode* visit(IRFunctionDefinition* node) override;
    virtual IRNode* visit(IRSourceFile* node) override;

private:
    template<typename Tr, typename Tv>
    Tr* checked_cast(Tv* value)
    {
        auto ptr = dynamic_cast<Tr*>(value);
        if (!ptr)
            throw std::exception("bad check_cast");
        return ptr;
    }
};