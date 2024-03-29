#pragma once
#include "../data/source_ref.hpp"
#include "../util/metadata_prop.hpp"
#include "../util/visitor.hpp"

using IRTripleDispatchVisitor = TDV::TripleDispatchVisitor<
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
    struct IRLoweredCallExpression,
    struct IRBoundCallExpression,

    struct IRStatement,
    struct IRBlockStatement,
    struct IRExpressionStatement,
    struct IRVariableStatement,
    struct IRReturnStatement,
    struct IRWhileStatement,
    struct IRIfStatement,

    struct IRConstraint,
    struct IRConstraintTemplate,
    struct IRConstraintInstantiation,
    struct IRFunction,
    struct IRConstraintFunction,
    struct IRIntrinsicFunction,
    struct IRNormalFunction,
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

template<typename TReturn, typename TRefBase = IRNode>
using IRVisitor = IRTripleDispatchVisitor::Visitor<TReturn, TRefBase>;

struct IRNode : IRTripleDispatchVisitor::NodeBase
{
    IMPLEMENT_ACCEPT()

    METADATA_PROP(location, SourceRef, getLocation, setLocation)
};