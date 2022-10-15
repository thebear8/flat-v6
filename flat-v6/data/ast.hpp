#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <typeinfo>
#include <optional>
#include <new>
#include <type_traits>

#include "token.hpp"
#include "operator.hpp"
#include "../type/type.hpp"
#include "../util/type_id.hpp"

namespace visitor
{
	template<typename ReturnType, typename... Types>
	struct VisitorImpl;

	template<typename ReturnType, typename T>
	struct VisitorImpl<ReturnType, T>
	{
		virtual ReturnType visit(T* visitable) = 0;
	};

	template<typename ReturnType, typename T, typename... Types>
	struct VisitorImpl<ReturnType, T, Types...> : public VisitorImpl<ReturnType, Types...>
	{
		using VisitorImpl<ReturnType, Types...>::visit;
		virtual ReturnType visit(T* visitable) = 0;
	};

	template<typename ReturnType, typename... Types>
	struct Visitor : VisitorImpl<ReturnType, Types...>
	{
		template<typename T>
		ReturnType dispatch(T* node) { return node->accept(this); }
	};
}

namespace typeid_visitor
{
	struct AstNode
	{
		virtual ~AstNode() {}
	};

	template<typename TVisitor, typename TReturn, typename TValue, typename... TNodes>
	struct DispatchImpl;

	template<typename TVisitor, typename TReturn, typename TValue, typename TFirst>
	struct DispatchImpl<TVisitor, TReturn, TValue, TFirst>
	{
		static __forceinline TReturn dispatch(TVisitor* visitor, TValue* node)
		{
			if (typeid(*node) == typeid(TFirst))
				return visitor->visit(reinterpret_cast<TFirst*>(node));
			throw std::exception();
		}
	};

	template<typename TVisitor, typename TReturn, typename TValue, typename TFirst, typename... TRest>
	struct DispatchImpl<TVisitor, TReturn, TValue, TFirst, TRest...>
	{
		static __forceinline TReturn dispatch(TVisitor* visitor, TValue* node)
		{
			if (typeid(*node) == typeid(TFirst))
				return visitor->visit(reinterpret_cast<TFirst*>(node));
			return DispatchImpl<TVisitor, TReturn, TValue, TRest...>::dispatch(visitor, node);
		}
	};

	template<typename TReturn, typename... TNodes>
	struct VisitorImpl;

	template<typename TReturn, typename TFirst>
	struct VisitorImpl<TReturn, TFirst>
	{
		virtual TReturn visit(TFirst* node) = 0;
	};

	template<typename TReturn, typename TFirst, typename... TRest>
	struct VisitorImpl<TReturn, TFirst, TRest...> : public VisitorImpl<TReturn, TRest...>
	{
		using VisitorImpl<TReturn, TRest...>::visit;
		virtual TReturn visit(TFirst* node) = 0;
	};

	template<typename TReturn, typename... TNodes>
	struct Visitor : public VisitorImpl<TReturn, TNodes...>
	{
		template<typename TValue>
		inline TReturn dispatch(TValue* node)
		{
			return DispatchImpl<VisitorImpl<TReturn, TNodes...>, TReturn, TValue, TNodes...>::dispatch(this, node);
		}
	};
}

namespace dynamic_visitor
{
	struct AstNode
	{
	private:
		TypeId type;

	public:
		virtual ~AstNode() = default;

	public:
		inline TypeId getType() { return type; }
		inline TypeId setType(TypeId value) { return (type = value); }
	};

	template<typename TVisitor, typename TReturn, typename... TNodes>
	struct DispatchImpl;

	template<typename TVisitor, typename TReturn, typename TFirst>
	struct DispatchImpl<TVisitor, TReturn, TFirst>
	{
		static inline TReturn dispatch(TVisitor* visitor, AstNode* node)
		{
			if (node->getType() == type_id<TFirst>())
				return visitor->visit(reinterpret_cast<TFirst*>(node));
			throw std::exception();
		}
	};

	template<typename TVisitor, typename TReturn, typename TFirst, typename... TRest>
	struct DispatchImpl<TVisitor, TReturn, TFirst, TRest...>
	{
		static inline TReturn dispatch(TVisitor* visitor, AstNode* node)
		{
			if (node->getType() == type_id<TFirst>())
				return visitor->visit(reinterpret_cast<TFirst*>(node));
			return DispatchImpl<TVisitor, TReturn, TRest...>::dispatch(visitor, node);
		}
	};

	template<typename TReturn, typename... TNodes>
	struct VisitorImpl;

	template<typename TReturn, typename TFirst>
	struct VisitorImpl<TReturn, TFirst>
	{
		virtual TReturn visit(TFirst* node) = 0;
	};

	template<typename TReturn, typename TFirst, typename... TRest>
	struct VisitorImpl<TReturn, TFirst, TRest...> : public VisitorImpl<TReturn, TRest...>
	{
		using VisitorImpl<TReturn, TRest...>::visit;
		virtual TReturn visit(TFirst* node) = 0;
	};

	template<typename TReturn, typename... TNodes>
	struct Visitor : public VisitorImpl<TReturn, TNodes...>
	{
		template<typename TValue>
		inline TReturn dispatch(TValue* node)
		{
			return DispatchImpl<VisitorImpl<TReturn, TNodes...>, TReturn, TValue, TNodes...>::dispatch(this, node);
		}
	};
}

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
			VisitorBase() { }

			VisitorBase(VisitorBase&&) = delete;
			VisitorBase(VisitorBase const&) = delete;
			VisitorBase& operator=(VisitorBase const&) = delete;

			~VisitorBase() { if (valid_) (*std::launder((TReturn*)&this->result_)).~TReturn(); }

		public:
			virtual TReturn visit(TFirst* node) = 0;
			virtual void invoke(TFirst* node) { this->valid_ = true; ::new (&this->result_) TReturn(visit(node)); }
		};

		template<typename TVisitInvoker, typename TFirst>
		struct VisitorBase<TVisitInvoker, void, TFirst> : public TVisitInvoker
		{
		public:
			virtual void visit(TFirst* node) = 0;
			virtual void invoke(TFirst* node) { return visit(node); }
		};

		template<typename TVisitInvoker, typename TReturn, typename TFirst, typename... TRest>
		struct VisitorBase<TVisitInvoker, TReturn, TFirst, TRest...> : public VisitorBase<TVisitInvoker, TReturn, TRest...>
		{
		public:
			using VisitorBase<TVisitInvoker, TReturn, TRest...>::visit;
			virtual TReturn visit(TFirst* node) = 0;

			using VisitorBase<TVisitInvoker, TReturn, TRest...>::invoke;
			virtual void invoke(TFirst* node) { this->valid_ = true; ::new (std::launder((TReturn*)&this->result_)) TReturn(visit(node)); }
		};

		template<typename TVisitInvoker, typename TFirst, typename... TRest>
		struct VisitorBase<TVisitInvoker, void, TFirst, TRest...> : VisitorBase<TVisitInvoker, void, TRest...>
		{
		public:
			using VisitorBase<TVisitInvoker, void, TRest...>::visit;
			virtual void visit(TFirst* node) = 0;

			using VisitorBase<TVisitInvoker, void, TRest...>::invoke;
			virtual void invoke(TFirst* node) { return visit(node); }
		};

		template<typename... TNodes>
		struct AstNode
		{
			using VisitInvoker = detail::VisitInvoker<TNodes...>;
			virtual void accept(VisitInvoker* visitor) = 0;
		};

		template<typename TReturn, typename... TNodes>
		struct Visitor : public detail::VisitorBase<detail::VisitInvoker<TNodes...>, TReturn, TNodes...>
		{
			TReturn dispatch(AstNode<TNodes...>* node)
			{
				node->accept(this);
				return *std::launder((TReturn*)&this->result_);
			}
		};

		template<typename... TNodes>
		struct Visitor<void, TNodes...> : public detail::VisitorBase<detail::VisitInvoker<TNodes...>, void, TNodes...>
		{
			void dispatch(AstNode<TNodes...>* node)
			{
				return node->accept(this);
			}
		};
	}

	template<typename... TNodes>
	struct TripleDispatchVisitor
	{
		using AstNode = detail::AstNode<TNodes...>;

		template<typename TReturn> 
		using Visitor = detail::Visitor<TReturn, TNodes...>;
	};
}

/*
template<typename ReturnType>
using Visitor = visitor::Visitor < ReturnType,
	struct AstNode,
	struct Expression,
	struct Statement,
	struct Declaration,
	struct IntegerExpression,
	struct BoolExpression,
	struct CharExpression,
	struct StringExpression,
	struct IdentifierExpression,
	struct StructExpression,
	struct UnaryExpression,
	struct BinaryExpression,
	struct CallExpression,
	struct BoundCallExpression,
	struct IndexExpression,
	struct BoundIndexExpression,
	struct FieldExpression,
	struct BlockStatement,
	struct ExpressionStatement,
	struct VariableStatement,
	struct ReturnStatement,
	struct WhileStatement,
	struct IfStatement,
	struct StructDeclaration,
	struct FunctionDeclaration,
	struct ExternFunctionDeclaration,
	struct Module
> ;

namespace llvm { struct Value; }

#define IMPLEMENT_ACCEPT_T(ReturnType) virtual ReturnType accept(Visitor<ReturnType>* visitor) { return visitor->visit(this); }
#define IMPLEMENT_ACCEPT() IMPLEMENT_ACCEPT_T(void) IMPLEMENT_ACCEPT_T(Type*) IMPLEMENT_ACCEPT_T(AstNode*) IMPLEMENT_ACCEPT_T(std::string) IMPLEMENT_ACCEPT_T(llvm::Value*)
*/

/*
template<typename ReturnType>
using Visitor = return_state_visitor::Visitor < ReturnType,
	struct AstNode,
	struct Expression,
	struct Statement,
	struct Declaration,
	struct IntegerExpression,
	struct BoolExpression,
	struct CharExpression,
	struct StringExpression,
	struct IdentifierExpression,
	struct StructExpression,
	struct UnaryExpression,
	struct BinaryExpression,
	struct CallExpression,
	struct BoundCallExpression,
	struct IndexExpression,
	struct BoundIndexExpression,
	struct FieldExpression,
	struct BlockStatement,
	struct ExpressionStatement,
	struct VariableStatement,
	struct ReturnStatement,
	struct WhileStatement,
	struct IfStatement,
	struct StructDeclaration,
	struct FunctionDeclaration,
	struct ExternFunctionDeclaration,
	struct Module
> ;

using AstNodeBase = return_state_visitor::AstNodeBase<
	struct AstNode,
	struct Expression,
	struct Statement,
	struct Declaration,
	struct IntegerExpression,
	struct BoolExpression,
	struct CharExpression,
	struct StringExpression,
	struct IdentifierExpression,
	struct StructExpression,
	struct UnaryExpression,
	struct BinaryExpression,
	struct CallExpression,
	struct BoundCallExpression,
	struct IndexExpression,
	struct BoundIndexExpression,
	struct FieldExpression,
	struct BlockStatement,
	struct ExpressionStatement,
	struct VariableStatement,
	struct ReturnStatement,
	struct WhileStatement,
	struct IfStatement,
	struct StructDeclaration,
	struct FunctionDeclaration,
	struct ExternFunctionDeclaration,
	struct Module
>;

#define IMPLEMENT_ACCEPT() virtual void accept(VisitorBase* visitor) { visitor->invokeVisit(this); }
*/

using TripleDispatchVisitor = triple_dispatch_visitor::TripleDispatchVisitor<
	struct AstNode,
	struct Expression,
	struct Statement,
	struct Declaration,
	struct IntegerExpression,
	struct BoolExpression,
	struct CharExpression,
	struct StringExpression,
	struct IdentifierExpression,
	struct StructExpression,
	struct UnaryExpression,
	struct BinaryExpression,
	struct CallExpression,
	struct BoundCallExpression,
	struct IndexExpression,
	struct BoundIndexExpression,
	struct FieldExpression,
	struct BlockStatement,
	struct ExpressionStatement,
	struct VariableStatement,
	struct ReturnStatement,
	struct WhileStatement,
	struct IfStatement,
	struct StructDeclaration,
	struct FunctionDeclaration,
	struct ExternFunctionDeclaration,
	struct Module
>;
using AstNodeBase = TripleDispatchVisitor::AstNode;
template<typename TReturn> using Visitor = TripleDispatchVisitor::Visitor<TReturn>;

#define IMPLEMENT_ACCEPT() virtual void accept(VisitInvoker* visitor) { visitor->invoke(this); }

struct AstNode : public AstNodeBase
{
	size_t begin, end;

	AstNode() :
		begin(0), end(0) { }

	IMPLEMENT_ACCEPT()
};

struct Declaration : public AstNode
{
	IMPLEMENT_ACCEPT()
};

struct Statement : public AstNode
{
	IMPLEMENT_ACCEPT()
};

struct Expression : public AstNode
{
	Type* type;

	Expression() :
		type(nullptr) { }

	IMPLEMENT_ACCEPT()
};

///////////////////////////////////////////

struct IntegerExpression : public Expression
{
	std::string value;
	std::string suffix;

	IntegerExpression(std::string const& value, std::string const& suffix) :
		value(value), suffix(suffix) { }

	IMPLEMENT_ACCEPT()
};

struct BoolExpression : public Expression
{
	std::string value;

	BoolExpression(std::string const& value) :
		value(value) { }

	IMPLEMENT_ACCEPT()
};

struct CharExpression : public Expression
{
	std::string value;

	CharExpression(std::string const& value) :
		value(value) { }

	IMPLEMENT_ACCEPT()
};

struct StringExpression : public Expression
{
	std::string value;

	StringExpression(std::string const& value) :
		value(value) { }

	IMPLEMENT_ACCEPT()
};

struct IdentifierExpression : public Expression
{
	std::string value;

	IdentifierExpression(std::string const& value) :
		value(value) { }

	IMPLEMENT_ACCEPT()
};

struct StructExpression : public Expression
{
	std::string structName;
	std::vector<std::pair<std::string, Expression*>> fields;

	StructExpression(std::string const& structName, std::vector<std::pair<std::string, Expression*>> const& fields) :
		structName(structName), fields(fields) { }

	IMPLEMENT_ACCEPT()
};

struct UnaryExpression : public Expression
{
	UnaryOperator operation;
	Expression* expression;

	UnaryExpression(UnaryOperator operation, Expression* expression) :
		operation(operation), expression(expression) { }

	IMPLEMENT_ACCEPT()
};

struct BinaryExpression : public Expression
{
	BinaryOperator operation;
	Expression* left, *right;

	BinaryExpression(BinaryOperator operation, Expression* left, Expression* right) :
		operation(operation), left(left), right(right) { }

	IMPLEMENT_ACCEPT()
};

struct CallExpression : public Expression
{
	Expression* expression;
	std::vector<Expression*> args;

	CallExpression(Expression* expression, std::vector<Expression*> const& args) :
		expression(expression), args(args) { }

	IMPLEMENT_ACCEPT()
};

struct BoundCallExpression : public Expression
{
	std::string identifier;
	std::vector<Expression*> args;

	BoundCallExpression(std::string identifier, std::vector<Expression*> const& args) :
		identifier(identifier), args(args) { }

	IMPLEMENT_ACCEPT()
};

struct IndexExpression : public Expression
{
	Expression* expression;
	std::vector<Expression*> args;

	IndexExpression(Expression* expression, std::vector<Expression*> const& args) :
		expression(expression), args(args) { }

	IMPLEMENT_ACCEPT()
};

struct BoundIndexExpression : public Expression
{
	Expression* expression;
	Expression* index;

	BoundIndexExpression(Expression* expression, Expression* index) :
		expression(expression), index(index) { }

	IMPLEMENT_ACCEPT()
};

struct FieldExpression : public Expression
{
	Expression* expression;
	std::string fieldName;

	FieldExpression(Expression* expression, std::string const& fieldName) :
		expression(expression), fieldName(fieldName) { }

	IMPLEMENT_ACCEPT()
};

///////////////////////////////////////////

struct BlockStatement : public Statement
{
	std::vector<Statement*> statements;

	BlockStatement(std::vector<Statement*> statements) :
		statements(statements) { }

	IMPLEMENT_ACCEPT()
};

struct ExpressionStatement : public Statement
{
	Expression* expression;

	ExpressionStatement(Expression* expression) :
		expression(expression) { }

	IMPLEMENT_ACCEPT()
};

struct VariableStatement : public Statement
{
	std::vector<std::pair<std::string, Expression*>> items;

	VariableStatement(std::vector<std::pair<std::string, Expression*>> const& items) :
		items(items) { }

	IMPLEMENT_ACCEPT()
};

struct ReturnStatement : public Statement
{
	Expression* expression;

	ReturnStatement(Expression* expression) :
		expression(expression) { }

	IMPLEMENT_ACCEPT()
};

struct WhileStatement : public Statement
{
	Expression* condition;
	Statement* body;

	WhileStatement(Expression* condition, Statement* body) :
		condition(condition), body(body) { }

	IMPLEMENT_ACCEPT()
};

struct IfStatement : public Statement
{
	Expression* condition;
	Statement* ifBody, * elseBody;

	IfStatement(Expression* condition, Statement* ifBody, Statement* elseBody) :
		condition(condition), ifBody(ifBody), elseBody(elseBody) { }

	IMPLEMENT_ACCEPT()
};

///////////////////////////////////////////

struct StructDeclaration : public Declaration
{
	std::string name;
	std::vector<std::pair<std::string, Type*>> fields;

	StructDeclaration(std::string const& name, std::vector<std::pair<std::string, Type*>> const& fields) :
		name(name), fields(fields) { }

	IMPLEMENT_ACCEPT()
};

struct FunctionDeclaration : public Declaration
{
	std::string name;
	Type* result;
	std::vector<std::pair<std::string, Type*>> parameters;
	Statement* body;

	FunctionDeclaration(std::string const& name, Type* result, std::vector<std::pair<std::string, Type*>> const& parameters, Statement* body) :
		name(name), result(result), parameters(parameters), body(body) { }

	IMPLEMENT_ACCEPT()
};

struct ExternFunctionDeclaration : public Declaration
{
	std::string lib;
	std::string name;
	Type* result;
	std::vector<std::pair<std::string, Type*>> parameters;

	ExternFunctionDeclaration(std::string const& lib, std::string const& name, Type* result, std::vector<std::pair<std::string, Type*>> const& parameters) :
		lib(lib), name(name), result(result), parameters(parameters) { }

	IMPLEMENT_ACCEPT()
};

///////////////////////////////////////////

struct Module : public AstNode
{
	std::vector<Declaration*> declarations;

	Module(std::vector<Declaration*> declarations) :
		declarations(declarations) { }

	IMPLEMENT_ACCEPT()
};

///////////////////////////////////////////

class AstContext
{
private:
	std::vector<AstNode*> nodes;

public:
	AstContext() { }
	AstContext(AstContext&&) = delete;
	AstContext(AstContext const&) = delete;

	~AstContext()
	{
		for (auto node : nodes)
			delete node;
		nodes.clear();
	}

	inline void deallocate()
	{
		for (auto node : nodes)
			delete node;
		nodes.clear();
	}

public:
	template<typename TNode, typename... TArgs>
	TNode* make(size_t begin, size_t end, TArgs&&... args)
	{
		nodes.push_back(new TNode(std::forward<TArgs>(args)...));
		//nodes.back()->setType(type_id<TNode>());
		nodes.back()->begin = begin;
		nodes.back()->end = end;
		return static_cast<TNode*>(nodes.back());
	}
};