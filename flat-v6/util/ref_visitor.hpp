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
};

template<typename TVisitInvoker, typename TReturn, typename... TNodes>
struct VisitorBase : VisitorBase<TVisitInvoker, TReturn, TNodes>...
{
    VisitorBase() {}

    VisitorBase(VisitorBase&&) = delete;
    VisitorBase(VisitorBase const&) = delete;
    VisitorBase& operator=(VisitorBase const&) = delete;
};

template<typename TVisitInvoker>
struct VisitorBase<TVisitInvoker, void> : TVisitInvoker
{
protected:
    void** m_node = nullptr;
};

template<typename TVisitInvoker, typename TReturn>
    requires std::movable<TReturn>
struct VisitorBase<TVisitInvoker, TReturn> : TVisitInvoker
{
protected:
    void** m_node = nullptr;
    std::aligned_storage<sizeof(TReturn), alignof(TReturn)>::type m_result = {};
};

template<typename TVisitInvoker, typename TNode>
struct VisitorBase<TVisitInvoker, void, TNode>
    : virtual VisitorBase<TVisitInvoker, void>
{
    virtual void visit(TNode*& node) { FLC_ASSERT(false); }

    virtual void invoke(TNode* node) override
    {
        auto& ref = *((TNode**)this->m_node);
        visit(ref);
    }
};

template<typename TVisitInvoker, typename TReturn, typename TNode>
struct VisitorBase<TVisitInvoker, TReturn, TNode>
    : virtual VisitorBase<TVisitInvoker, TReturn>
{
    virtual TReturn visit(TNode*& node) { FLC_ASSERT(false); }

    virtual void invoke(TNode* node) override
    {
        auto& ref = *((TNode**)this->m_node);
        ::new (&this->m_result) TReturn(std::forward<TReturn>(visit(ref)));
    }
};

template<typename... TNodes>
struct NodeBase
{
    virtual ~NodeBase() {}

    using VisitInvoker = ::TDV::VisitInvoker<TNodes...>;
    virtual void accept(VisitInvoker* visitor) = 0;
};

template<typename TReturn, typename... TNodes>
struct Visitor : VisitorBase<VisitInvoker<TNodes...>, TReturn, TNodes...>
{
    template<typename TNode>
        requires std::derived_from<TNode, NodeBase<TNodes...>>
    TReturn dispatch(TNode*& node)
    {
        this->m_node = (void**)(&node);
        node->accept(this);
        this->m_node = nullptr;
        return std::move(*std::launder((TReturn*)&this->m_result));
    }
};

template<typename... TNodes>
struct Visitor<void, TNodes...>
    : VisitorBase<VisitInvoker<TNodes...>, void, TNodes...>
{
    template<typename TNode>
        requires std::derived_from<TNode, NodeBase<TNodes...>>
    void dispatch(TNode*& node)
    {
        this->m_node = (void**)(&node);
        node->accept(this);
        this->m_node = nullptr;
    }
};

template<typename... TNodes>
struct TripleDispatchVisitor
{
    using NodeBase = ::TDV::NodeBase<TNodes...>;

    template<typename TReturn>
    using Visitor = ::TDV::Visitor<TReturn, TNodes...>;
};
}

#define IMPLEMENT_ACCEPT()                     \
    virtual void accept(VisitInvoker* visitor) \
    {                                          \
        visitor->invoke(this);                 \
    }
