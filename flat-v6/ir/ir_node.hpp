#pragma once
#include "../data/source_ref.hpp"
#include "../util/metadata_prop.hpp"
#include "../util/visitor.hpp"

using IRTripleDispatchVisitor = triple_dispatch_visitor::TripleDispatchVisitor<
    struct IRNode,

    struct IRExpression,
    struct IRIntegerExpression,
    struct IRBoolExpression,
    struct IRCharExpression,
    struct IRStringExpression,
    struct IRIdentifierExpression,
    struct IRStructExpression,
    struct IRUnaryExpression,
    struct IRBinaryExpression,
    struct IRCallExpression,
    struct IRIndexExpression,
    struct IRFieldExpression,
    struct IRBlockStatement,

    struct IRStatement,
    struct IRExpressionStatement,
    struct IRVariableStatement,
    struct IRReturnStatement,
    struct IRWhileStatement,
    struct IRIfStatement,

    struct IRConstraint,
    struct IRFunction,
    struct IRModule,

    struct IRType,
    struct IRGenericType,
    struct IRVoidType,
    struct IRBoolType,
    struct IRIntegerType,
    struct IRCharType,
    struct IRStringType,
    struct IRStructType,
    struct IRPointerType,
    struct IRArrayType>;

template<typename TReturn>
using IRVisitor = IRTripleDispatchVisitor::Visitor<TReturn>;

class Environment;
class GraphContext;

struct IRNode : IRTripleDispatchVisitor::NodeBase
{
    IMPLEMENT_ACCEPT()

    METADATA_PROP(location, SourceRef, getLocation, setLocation)
    METADATA_PROP(type, IRType*, getType, setType)
    METADATA_PROP(target, IRFunction*, getTarget, setTarget)
    METADATA_PROP(irCtx, GraphContext*, getIrCtx, setIrCtx)
    METADATA_PROP(env, Environment*, getEnv, setEnv)
};