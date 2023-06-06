#pragma once
#include <concepts>
#include <new>
#include <type_traits>

#include "assert.hpp"

namespace TDV
{
template<typename... TNodes>
struct VisitInvoker : VisitInvoker<TNodes>...
{
    using VisitInvoker<TNodes>::invoke...;
};

template<typename TNode>
struct VisitInvoker<TNode>
{
    virtual void invoke(TNode* node) = 0;
    virtual void invoke(TNode* node, void** ref) = 0;
};

template<typename... TNodes>
struct NodeBase
{
    virtual ~NodeBase() {}

    using VisitInvoker = ::TDV::VisitInvoker<TNodes...>;
    virtual void accept(VisitInvoker* visitor) = 0;
    virtual void accept(VisitInvoker* visitor, void** ref) = 0;
};

//

template<
    typename TVisitInvoker,
    typename TReturn,
    typename TRefBase,
    typename... TNodes>
struct VisitorBase : VisitorBase<TVisitInvoker, TReturn, TRefBase, TNodes>...
{
    VisitorBase() {}

    VisitorBase(VisitorBase&&) = delete;
    VisitorBase(VisitorBase const&) = delete;
    VisitorBase& operator=(VisitorBase const&) = delete;
};

template<typename TVisitInvoker, typename TReturn, typename TRefBase>
    requires std::movable<TReturn>
struct VisitorBase<TVisitInvoker, TReturn, TRefBase> : TVisitInvoker
{
protected:
    std::aligned_storage<sizeof(TReturn), alignof(TReturn)>::type m_result = {};
};

template<typename TVisitInvoker, typename TRefBase>
struct VisitorBase<TVisitInvoker, void, TRefBase> : TVisitInvoker
{
};

template<
    typename TVisitInvoker,
    typename TReturn,
    typename TRefBase,
    typename TNode>
struct VisitorBase<TVisitInvoker, TReturn, TRefBase, TNode>
    : virtual VisitorBase<TVisitInvoker, TReturn, TRefBase>
{
    virtual TReturn visit(TNode* node)
    {
        FLC_ASSERT(false);
        return {};
    }
    virtual TReturn visit(TNode* node, TRefBase*& ref) { return visit(node); }

    virtual void invoke(TNode* node) override
    {
        ::new (&this->m_result) TReturn(std::forward<TReturn>(visit(node)));
    }

    virtual void invoke(TNode* node, void** ref) override
    {
        ::new (&this->m_result)
            TReturn(std::forward<TReturn>(visit(node, *((TRefBase**)ref))));
    }
};

template<typename TVisitInvoker, typename TRefBase, typename TNode>
struct VisitorBase<TVisitInvoker, void, TRefBase, TNode>
    : virtual VisitorBase<TVisitInvoker, void, TRefBase>
{
    virtual void visit(TNode* node) { FLC_ASSERT(false); }
    virtual void visit(TNode* node, TRefBase*& ref) { return visit(node); }

    virtual void invoke(TNode* node) override { return visit(node); }

    virtual void invoke(TNode* node, void** ref) override
    {
        return visit(node, *((TRefBase**)ref));
    }
};

template<typename TReturn, typename TRefBase, typename... TNodes>
struct Visitor
    : VisitorBase<VisitInvoker<TNodes...>, TReturn, TRefBase, TNodes...>
{
    template<typename TNode>
        requires std::derived_from<TNode, NodeBase<TNodes...>>
    TReturn dispatch(TNode* node)
    {
        node->accept(this);
        return std::move(*std::launder((TReturn*)&this->m_result));
    }

    template<typename TNode>
        requires std::derived_from<TNode, TRefBase>
    TReturn dispatchRef(TNode*& node)
    {
        node->accept(this, (void**)&node);
        return std::move(*std::launder((TReturn*)&this->m_result));
    }
};

template<typename TRefBase, typename... TNodes>
struct Visitor<void, TRefBase, TNodes...>
    : VisitorBase<VisitInvoker<TNodes...>, void, TRefBase, TNodes...>
{
    template<typename TNode>
        requires std::derived_from<TNode, NodeBase<TNodes...>>
    void dispatch(TNode* node)
    {
        return node->accept(this);
    }

    template<typename TNode>
        requires std::derived_from<TNode, TRefBase>
    void dispatchRef(TNode*& node)
    {
        return node->accept(this, (void**)&node);
    }
};

//

template<typename... TNodes>
struct TripleDispatchVisitor
{
    using NodeBase = ::TDV::NodeBase<TNodes...>;

    template<typename TReturn, typename TRefBase>
    using Visitor = ::TDV::Visitor<TReturn, TRefBase, TNodes...>;
};
}

#define IMPLEMENT_ACCEPT()                                 \
    virtual void accept(VisitInvoker* visitor)             \
    {                                                      \
        visitor->invoke(this);                             \
    }                                                      \
    virtual void accept(VisitInvoker* visitor, void** ref) \
    {                                                      \
        visitor->invoke(this, ref);                        \
    }
