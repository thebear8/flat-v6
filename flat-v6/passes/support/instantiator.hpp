#include "../../compiler.hpp"
#include "../../ir/ir.hpp"
#include "../../util/error_logger.hpp"
#include "../../util/graph_context.hpp"

class Instantiator : IRVisitor<IRNode*>
{
private:
    Environment* m_env;
    GraphContext* m_irCtx;

public:
    Instantiator() : m_env(nullptr), m_irCtx(nullptr) {}

    IRStructInstantiation* makeStructInstantiation(
        IRModule* irModule,
        IRStructTemplate* structTemplate,
        std::vector<IRType*> const& typeArgs
    );

    IRFunctionInstantiation* makeFunctionInstantiation(
        IRModule* irModule,
        IRFunctionTemplate* functionTemplate,
        std::vector<IRType*> const& typeArgs
    );

private:
    IRNode* visit(IRIntegerExpression* node) override { return node; }
    IRNode* visit(IRBoolExpression* node) override { return node; }
    IRNode* visit(IRCharExpression* node) override { return node; }
    IRNode* visit(IRStringExpression* node) override { return node; }
    IRNode* visit(IRIdentifierExpression* node) override;
    IRNode* visit(IRStructExpression* node) override;
    IRNode* visit(IRUnaryExpression* node) override;
    IRNode* visit(IRBinaryExpression* node) override;
    IRNode* visit(IRCallExpression* node) override;
    IRNode* visit(IRIndexExpression* node) override;
    IRNode* visit(IRFieldExpression* node) override;

    IRNode* visit(IRBlockStatement* node) override;
    IRNode* visit(IRExpressionStatement* node) override;
    IRNode* visit(IRVariableStatement* node) override;
    IRNode* visit(IRReturnStatement* node) override;
    IRNode* visit(IRWhileStatement* node) override;
    IRNode* visit(IRIfStatement* node) override;

    IRNode* visit(IRGenericType* node) override;
    IRNode* visit(IRVoidType* node) override { return node; }
    IRNode* visit(IRBoolType* node) override { return node; }
    IRNode* visit(IRIntegerType* node) override { return node; }
    IRNode* visit(IRCharType* node) override { return node; }
    IRNode* visit(IRStringType* node) override { return node; }
    IRNode* visit(IRPointerType* node) override;
    IRNode* visit(IRArrayType* node) override;
    IRNode* visit(IRStructInstantiation* node) override;
};