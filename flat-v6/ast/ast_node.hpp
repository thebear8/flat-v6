#pragma once
#include "../data/source_ref.hpp"
#include "../util/metadata_prop.hpp"
#include "../util/visitor.hpp"

using ASTTripleDispatchVisitor = triple_dispatch_visitor::TripleDispatchVisitor<
    struct ASTNode,

    struct ASTExpression,
    struct ASTIntegerExpression,
    struct ASTBoolExpression,
    struct ASTCharExpression,
    struct ASTStringExpression,
    struct ASTIdentifierExpression,
    struct ASTStructExpression,
    struct ASTUnaryExpression,
    struct ASTBinaryExpression,
    struct ASTCallExpression,
    struct ASTIndexExpression,
    struct ASTFieldExpression,

    struct ASTStatement,
    struct ASTBlockStatement,
    struct ASTExpressionStatement,
    struct ASTVariableStatement,
    struct ASTReturnStatement,
    struct ASTWhileStatement,
    struct ASTIfStatement,

    struct ASTDeclaration,
    struct ASTRequirement,
    struct ASTConstraintCondition,
    struct ASTConstraintDeclaration,
    struct ASTStructDeclaration,
    struct ASTFunctionDeclaration,

    struct ASTType,
    struct ASTNamedType,
    struct ASTPointerType,
    struct ASTArrayType,

    struct ASTSourceFile>;

template<typename TReturn>
using ASTVisitor = ASTTripleDispatchVisitor::Visitor<TReturn>;

struct ASTNode : ASTTripleDispatchVisitor::NodeBase
{
    SourceRef location;

    ASTNode(SourceRef const& location) : location(location) {}

    IMPLEMENT_ACCEPT()
};