#pragma once
#include <string>
#include <tuple>

#include "../../ast/ast.hpp"

class IRType;
class Environment;
class GraphContext;
class StructInstantiator;

class ASTTypeResolver : ASTVisitor<std::tuple<IRType*, std::string>>
{
private:
    StructInstantiator& m_structInstantiator;

    Environment* m_env = nullptr;
    GraphContext* m_irCtx = nullptr;

public:
    ASTTypeResolver(StructInstantiator& structInstantiator)
        : m_structInstantiator(structInstantiator)
    {
    }

public:
    std::tuple<IRType*, std::string> resolve(
        ASTType* node, Environment* env, GraphContext* irCtx
    );

private:
    std::tuple<IRType*, std::string> visit(ASTNamedType* node) override;
    std::tuple<IRType*, std::string> visit(ASTPointerType* node) override;
    std::tuple<IRType*, std::string> visit(ASTArrayType* node) override;
};