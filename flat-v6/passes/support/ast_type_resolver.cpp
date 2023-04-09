#include "ast_type_resolver.hpp"

#include "../../environment.hpp"
#include "../../ir/ir.hpp"
#include "../../util/graph_context.hpp"

std::tuple<IRType*, std::string> ASTTypeResolver::resolve(
    ASTType* node, Environment* env, GraphContext* irCtx
)
{
    m_env = env;
    m_irCtx = irCtx;

    auto&& resolved = dispatch(node);

    m_env = nullptr;
    m_irCtx = nullptr;

    return resolved;
}

std::tuple<IRType*, std::string> ASTTypeResolver::visit(ASTNamedType* node)
{
    if (auto builtinType = m_env->findBuiltinType(node->name))
    {
        return std::make_tuple(builtinType, "");
    }
    else if (auto typeParam = m_env->findTypeParam(node->name))
    {
        return std::make_tuple(typeParam, "");
    }
    if (auto structTemplate = m_env->findStruct(node->name);
        structTemplate || node->typeArgs.size() != 0)
    {
        if (!structTemplate)
            return std::make_tuple(nullptr, "No struct named " + node->name);

        if (structTemplate->typeParams.size() != node->typeArgs.size())
        {
            return std::make_tuple(
                nullptr,
                "Number of type arguments for struct " + node->name
                    + " does not match number of type parameters"
            );
        }

        std::vector<IRType*> typeArgs;
        for (auto typeArg : node->typeArgs)
        {
            auto&& [type, error] = dispatch(typeArg);
            if (!type)
                return std::make_tuple(nullptr, error);
            typeArgs.push_back(type);
        }

        auto env = structTemplate->getParent()->getEnv();
        auto instantiation =
            env->getStructInstantiation(structTemplate, typeArgs);

        if (instantiation)
        {
            return std::make_tuple(instantiation, "");
        }
        else
        {
            return std::make_tuple(
                m_instantiator.makeStructInstantiation(
                    structTemplate, typeArgs
                ),
                ""
            );
        }
    }
    else
    {
        return std::make_tuple(nullptr, "No type named " + node->name);
    }
}

std::tuple<IRType*, std::string> ASTTypeResolver::visit(ASTPointerType* node)
{
    auto&& [base, error] = dispatch(node->base);
    if (!base)
        return std::make_tuple(nullptr, error);
    return std::make_tuple(m_irCtx->make(IRPointerType(base)), "");
}

std::tuple<IRType*, std::string> ASTTypeResolver::visit(ASTArrayType* node)
{
    auto&& [base, error] = dispatch(node->base);
    if (!base)
        return std::make_tuple(nullptr, error);
    return std::make_tuple(m_irCtx->make(IRArrayType(base)), "");
}