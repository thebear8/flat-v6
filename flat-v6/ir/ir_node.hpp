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
    struct IRConstraintTemplate,
    struct IRConstraintInstantiation,
    struct IRFunction,
    struct IRFunctionTemplate,
    struct IRFunctionInstantiation,
    struct IRModule,

    struct IRType,
    struct IRGenericType,
    struct IRVoidType,
    struct IRBoolType,
    struct IRIntegerType,
    struct IRCharType,
    struct IRStringType,
    struct IRPointerType,
    struct IRArrayType,
    struct IRStruct,
    struct IRStructTemplate,
    struct IRStructInstantiation>;

template<typename TReturn>
using IRVisitor = IRTripleDispatchVisitor::Visitor<TReturn>;

struct IRNode : IRTripleDispatchVisitor::NodeBase
{
    IMPLEMENT_ACCEPT()

    METADATA_PROP(location, SourceRef, getLocation, setLocation)
};