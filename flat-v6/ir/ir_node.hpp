#include "../data/source_ref.hpp"
#include "../util/md_container.hpp"
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

    struct IRDeclaration,
    struct IRConstraintDeclaration,
    struct IRStructDeclaration,
    struct IRFunctionDeclaration,
    struct IRSourceFile,

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

using IRMetadataContainer = MetadataContainer<SourceRef, IRType*>;

struct IRNode : IRTripleDispatchVisitor::NodeBase, private IRMetadataContainer
{
    IMPLEMENT_ACCEPT()

    template<typename TValue>
    std::optional<TValue> const& getMD()
    {
        return IRMetadataContainer::setMD<TValue>();
    }

    template<typename TValue>
    IRNode* setMD(TValue&& v)
    {
        IRMetadataContainer::setMD<TValue>(std::forward<TValue>(v));
        return this;
    }

    template<typename TValue>
    IRNode* setMD(TValue const& v)
    {
        IRMetadataContainer::setMD<TValue>(std::forward<TValue>(v));
        return this;
    }
};