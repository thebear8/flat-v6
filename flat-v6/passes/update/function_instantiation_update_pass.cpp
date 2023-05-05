#include "function_instantiation_update_pass.hpp"

#include "../../environment.hpp"
#include "../../util/assert.hpp"
#include "../../util/graph_context.hpp"
#include "../../util/to_vector.hpp"
#include "../../util/zip_view.hpp"
#include "../support/call_target_resolver.hpp"
#include "../support/instantiator.hpp"

void FunctionInstantiationUpdatePass::process(IRModule* node)
{
    for (auto [functionTemplate, functionInstantiation] :
         node->getEnv()->getFunctionInstantiationMap())
    {
        update(functionInstantiation);
    }
}

IRFunctionInstantiation* FunctionInstantiationUpdatePass::update(
    IRFunctionInstantiation* functionInstantiation
)
{
    auto functionTemplate = functionInstantiation->getInstantiatedFrom();

    FLC_ASSERT(
        functionInstantiation->typeArgs.size()
            == functionTemplate->typeParams.size(),
        "Number of type args has to match number of type params"
    );

    m_env = m_envCtx.make(Environment(
        functionTemplate->name, functionTemplate->getParent()->getEnv()
    ));
    m_irCtx = functionTemplate->getParent()->getIrCtx();

    auto zippedTypeArgs = zip_view(
        std::views::all(functionTemplate->typeParams),
        std::views::all(functionInstantiation->typeArgs)
    );

    for (auto [typeParam, typeArg] : zippedTypeArgs)
        m_env->addTypeParamValue(typeParam, typeArg);

    functionInstantiation->body =
        (IRStatement*)dispatch(functionTemplate->body);

    m_env = nullptr;
    m_irCtx = nullptr;
    return functionInstantiation;
}

IRNode* FunctionInstantiationUpdatePass::visit(IRIdentifierExpression*& node)
{
    auto type = m_instantiator.instantiateType(node->getType(), m_env, m_irCtx);
    auto location = node->getLocation(SourceRef());
    auto typeArgs =
        node->typeArgs | std::views::transform([&](auto a) {
            return m_instantiator.instantiateType(a, m_env, m_irCtx);
        });

    return m_irCtx
        ->make(IRIdentifierExpression(
            node->value, std::vector(typeArgs.begin(), typeArgs.end())
        ))
        ->setType(type)
        ->setLocation(location);
}

IRNode* FunctionInstantiationUpdatePass::visit(IRStructExpression*& node)
{
    auto type = m_instantiator.instantiateType(node->getType(), m_env, m_irCtx);
    auto location = node->getLocation(SourceRef());
    auto typeArgs =
        node->typeArgs | std::views::transform([&](auto a) {
            return m_instantiator.instantiateType(a, m_env, m_irCtx);
        });
    auto fields =
        node->fields | std::views::transform([&](auto& f) {
            return std::pair(f.first, (IRExpression*)dispatch(f.second));
        });

    return m_irCtx
        ->make(IRStructExpression(
            node->structName,
            std::vector(typeArgs.begin(), typeArgs.end()),
            std::unordered_map(fields.begin(), fields.end())
        ))
        ->setType(type)
        ->setLocation(location);
}

IRNode* FunctionInstantiationUpdatePass::visit(IRUnaryExpression*& node)
{
    auto type = m_instantiator.instantiateType(node->getType(), m_env, m_irCtx);
    auto location = node->getLocation(SourceRef());
    auto expression = (IRExpression*)dispatch(node->expression);
    auto target = (IRFunctionInstantiation*)dispatch(node->getTarget());

    return m_irCtx->make(IRUnaryExpression(node->operation, expression))
        ->setTarget(target)
        ->setType(type)
        ->setLocation(location);
}

IRNode* FunctionInstantiationUpdatePass::visit(IRBinaryExpression*& node)
{
    auto type = m_instantiator.instantiateType(node->getType(), m_env, m_irCtx);
    auto location = node->getLocation(SourceRef());
    auto left = (IRExpression*)dispatch(node->left);
    auto right = (IRExpression*)dispatch(node->right);
    auto target = (IRFunctionInstantiation*)dispatch(node->getTarget());

    return m_irCtx->make(IRBinaryExpression(node->operation, left, right))
        ->setTarget(target)
        ->setType(type)
        ->setLocation(location);
}

IRNode* FunctionInstantiationUpdatePass::visit(IRCallExpression*& node)
{
    auto type = m_instantiator.instantiateType(node->getType(), m_env, m_irCtx);
    auto location = node->getLocation(SourceRef());
    auto expression = (IRExpression*)dispatch(node->expression);
    auto args = node->args | std::views::transform([&](auto a) {
                    return (IRExpression*)dispatch(a);
                });
    auto target = (IRFunctionInstantiation*)dispatch(node->getTarget());

    return m_irCtx
        ->make(
            IRCallExpression(expression, std::vector(args.begin(), args.end()))
        )
        ->setTarget(target)
        ->setType(type)
        ->setLocation(location);
}

IRNode* FunctionInstantiationUpdatePass::visit(IRIndexExpression*& node)
{
    auto type = m_instantiator.instantiateType(node->getType(), m_env, m_irCtx);
    auto location = node->getLocation(SourceRef());
    auto expression = (IRExpression*)dispatch(node->expression);
    auto args = node->args | std::views::transform([&](auto a) {
                    return (IRExpression*)dispatch(a);
                });
    auto target = (IRFunctionInstantiation*)dispatch(node->getTarget());

    return m_irCtx
        ->make(
            IRIndexExpression(expression, std::vector(args.begin(), args.end()))
        )
        ->setTarget(target)
        ->setType(type)
        ->setLocation(location);
}

IRNode* FunctionInstantiationUpdatePass::visit(IRFieldExpression*& node)
{
    auto type = m_instantiator.instantiateType(node->getType(), m_env, m_irCtx);
    auto expression = (IRExpression*)dispatch(node->expression);

    return m_irCtx->make(IRFieldExpression(expression, node->fieldName))
        ->setType(type)
        ->setLocation(node->getLocation(SourceRef()));
}

IRNode* FunctionInstantiationUpdatePass::visit(IRBlockStatement*& node)
{
    auto statements = node->statements | std::views::transform([&](auto s) {
                          return (IRStatement*)dispatch(s);
                      });

    return m_irCtx
        ->make(
            IRBlockStatement(std::vector(statements.begin(), statements.end()))
        )
        ->setLocation(node->getLocation(SourceRef()));
}

IRNode* FunctionInstantiationUpdatePass::visit(IRExpressionStatement*& node)
{
    auto expression = (IRExpression*)dispatch(node->expression);

    return m_irCtx->make(IRExpressionStatement(expression))
        ->setLocation(node->getLocation(SourceRef()));
}

IRNode* FunctionInstantiationUpdatePass::visit(IRVariableStatement*& node)
{
    auto items =
        node->items | std::views::transform([&](auto i) {
            return std::pair(i.first, (IRExpression*)dispatch(i.second));
        });

    return m_irCtx
        ->make(IRVariableStatement(std::vector(items.begin(), items.end())))
        ->setLocation(node->getLocation(SourceRef()));
}

IRNode* FunctionInstantiationUpdatePass::visit(IRReturnStatement*& node)
{
    auto expression = (IRExpression*)dispatch(node->expression);

    return m_irCtx->make(IRReturnStatement(expression))
        ->setLocation(node->getLocation(SourceRef()));
}

IRNode* FunctionInstantiationUpdatePass::visit(IRWhileStatement*& node)
{
    auto condition = (IRExpression*)dispatch(node->condition);
    auto body = (IRStatement*)dispatch(node->body);

    return m_irCtx->make(IRWhileStatement(condition, body))
        ->setLocation(node->getLocation(SourceRef()));
}

IRNode* FunctionInstantiationUpdatePass::visit(IRIfStatement*& node)
{
    auto condition = (IRExpression*)dispatch(node->condition);
    auto ifBody = (IRStatement*)dispatch(node->ifBody);
    auto elseBody =
        node->elseBody ? (IRStatement*)dispatch(node->elseBody) : nullptr;

    return m_irCtx->make(IRIfStatement(condition, ifBody, elseBody))
        ->setLocation(node->getLocation(SourceRef()));
}

IRNode* FunctionInstantiationUpdatePass::visit(IRFunctionHead*& node)
{
    auto args =
        node->params | std::views::transform([&](auto const& p) {
            return m_instantiator.instantiateType(p.second, m_env, m_irCtx);
        })
        | range_utils::to_vector;
    auto result = m_instantiator.instantiateType(node->result, m_env, m_irCtx);

    std::vector<IRType*> typeArgs;
    auto target = m_callTargetResolver.findMatchingFunctionTemplate(
        m_env, node->name, {}, args, result, typeArgs
    );
    FLC_ASSERT(target, "Target for constraint condition has to exist.");

    return m_instantiator.getFunctionInstantiation(target, typeArgs);
}

IRNode* FunctionInstantiationUpdatePass::visit(IRFunctionInstantiation*& node)
{
    auto typeArgs =
        node->typeArgs | std::views::transform([&](auto arg) {
            return m_instantiator.instantiateType(arg, m_env, m_irCtx);
        })
        | range_utils::to_vector;

    return m_instantiator.getFunctionInstantiation(
        node->getInstantiatedFrom(), typeArgs
    );
}