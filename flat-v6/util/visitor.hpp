#pragma once
#include <new>
#include <type_traits>
#include <typeinfo>

namespace triple_dispatch_visitor
{
namespace detail
{
template<typename... TNodes>
struct VisitInvoker;

template<typename TFirst>
struct VisitInvoker<TFirst>
{
    virtual void invoke(TFirst* node) = 0;
};

template<typename TFirst, typename... TRest>
struct VisitInvoker<TFirst, TRest...> : public VisitInvoker<TRest...>
{
    using VisitInvoker<TRest...>::invoke;
    virtual void invoke(TFirst* node) = 0;
};

template<typename TVisitInvoker, typename TReturn, typename... TNodes>
struct VisitorBase;

template<typename TVisitInvoker, typename TReturn, typename TFirst>
struct VisitorBase<TVisitInvoker, TReturn, TFirst> : public TVisitInvoker
{
protected:
    bool valid_ = false;
    std::aligned_storage<sizeof(TReturn), alignof(TReturn)>::type result_ = {};

public:
    VisitorBase() {}

    VisitorBase(VisitorBase&&) = delete;
    VisitorBase(VisitorBase const&) = delete;
    VisitorBase& operator=(VisitorBase const&) = delete;

    ~VisitorBase()
    {
        if (valid_)
            std::launder((TReturn*)&this->result_)->~TReturn();
    }

public:
    virtual TReturn visit(TFirst* node)
    {
        throw std::exception(
            (std::string("visit(") + typeid(TFirst).name() + ") unimplemented")
                .c_str());
    }
    virtual void invoke(TFirst* node)
    {
        this->valid_ = true;
        ::new (&this->result_) TReturn(visit(node));
    }
};

template<typename TVisitInvoker, typename TFirst>
struct VisitorBase<TVisitInvoker, void, TFirst> : public TVisitInvoker
{
public:
    virtual void visit(TFirst* node)
    {
        throw std::exception(
            (std::string("visit(") + typeid(TFirst).name() + ") unimplemented")
                .c_str());
    }
    virtual void invoke(TFirst* node) { return visit(node); }
};

template<
    typename TVisitInvoker,
    typename TReturn,
    typename TFirst,
    typename... TRest>
struct VisitorBase<TVisitInvoker, TReturn, TFirst, TRest...>
    : public VisitorBase<TVisitInvoker, TReturn, TRest...>
{
public:
    using VisitorBase<TVisitInvoker, TReturn, TRest...>::visit;
    virtual TReturn visit(TFirst* node)
    {
        throw std::exception(
            (std::string("visit(") + typeid(TFirst).name() + ") unimplemented")
                .c_str());
    }

    using VisitorBase<TVisitInvoker, TReturn, TRest...>::invoke;
    virtual void invoke(TFirst* node)
    {
        this->valid_ = true;
        ::new (std::launder((TReturn*)&this->result_)) TReturn(visit(node));
    }
};

template<typename TVisitInvoker, typename TFirst, typename... TRest>
struct VisitorBase<TVisitInvoker, void, TFirst, TRest...>
    : VisitorBase<TVisitInvoker, void, TRest...>
{
public:
    using VisitorBase<TVisitInvoker, void, TRest...>::visit;
    virtual void visit(TFirst* node)
    {
        throw std::exception(
            (std::string("visit(") + typeid(TFirst).name() + ") unimplemented")
                .c_str());
    }

    using VisitorBase<TVisitInvoker, void, TRest...>::invoke;
    virtual void invoke(TFirst* node) { return visit(node); }
};

template<typename... TNodes>
struct NodeBase
{
    virtual ~NodeBase() {}

    using VisitInvoker = detail::VisitInvoker<TNodes...>;
    virtual void accept(VisitInvoker* visitor) = 0;
};

template<typename TReturn, typename... TNodes>
struct Visitor
    : public detail::
          VisitorBase<detail::VisitInvoker<TNodes...>, TReturn, TNodes...>
{
    TReturn dispatch(NodeBase<TNodes...>* node)
    {
        node->accept(this);
        return *std::launder((TReturn*)&this->result_);
    }
};

template<typename... TNodes>
struct Visitor<void, TNodes...>
    : public detail::
          VisitorBase<detail::VisitInvoker<TNodes...>, void, TNodes...>
{
    void dispatch(NodeBase<TNodes...>* node) { return node->accept(this); }
};
}

template<typename... TNodes>
struct TripleDispatchVisitor
{
    using NodeBase = detail::NodeBase<TNodes...>;

    template<typename TReturn>
    using Visitor = detail::Visitor<TReturn, TNodes...>;
};
}

#define IMPLEMENT_ACCEPT()                     \
    virtual void accept(VisitInvoker* visitor) \
    {                                          \
        visitor->invoke(this);                 \
    }