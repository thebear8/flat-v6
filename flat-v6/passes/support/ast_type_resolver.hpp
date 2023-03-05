#pragma once
#include <string>
#include <tuple>

#include "../../ast/ast.hpp"
#include "instantiator.hpp"

class IRType;
class Environment;
class GraphContext;

class ASTTypeResolver : ASTVisitor<std::tuple<IRType*, std::string>>
{
private:
    Instantiator m_instantiator;

    Environment* m_env;
    GraphContext* m_irCtx;

public:
    ASTTypeResolver() : m_env(nullptr), m_irCtx(nullptr) {}

public:
    std::tuple<IRType*, std::string> resolve(
        ASTType* node, Environment* env, GraphContext* irCtx
    );

private:
    std::tuple<IRType*, std::string> visit(ASTNamedType* node) override;
    std::tuple<IRType*, std::string> visit(ASTPointerType* node) override;
    std::tuple<IRType*, std::string> visit(ASTArrayType* node) override;
};