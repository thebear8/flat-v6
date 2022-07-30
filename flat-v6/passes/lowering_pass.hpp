#pragma once
#include <string>
#include <string_view>
#include <ostream>

#include "../data/ast.hpp"
#include "../util/error_logger.hpp"
#include "../data/operator.hpp"

class OperatorLoweringPass : protected Visitor<AstNode*>, protected ErrorLogger
{
private:
	AstContext& astCtx;
	TypeContext& typeCtx;

public:
	OperatorLoweringPass(AstContext& astCtx, TypeContext& ctx, std::string_view source, std::ostream& logStream) :
		ErrorLogger(source, logStream), astCtx(astCtx), typeCtx(ctx) { }

public:
	AstNode* process(AstNode* program);

protected:
	virtual AstNode* visit(IntegerExpression* node) override;
	virtual AstNode* visit(BoolExpression* node) override;
	virtual AstNode* visit(CharExpression* node) override;
	virtual AstNode* visit(StringExpression* node) override;
	virtual AstNode* visit(IdentifierExpression* node) override;
	virtual AstNode* visit(StructExpression* node) override;
	virtual AstNode* visit(UnaryExpression* node) override;
	virtual AstNode* visit(BinaryExpression* node) override;
	virtual AstNode* visit(CallExpression* node) override;
	virtual AstNode* visit(IndexExpression* node) override;
	virtual AstNode* visit(FieldExpression* node) override;

	virtual AstNode* visit(BlockStatement* node) override;
	virtual AstNode* visit(ExpressionStatement* node) override;
	virtual AstNode* visit(VariableStatement* node) override;
	virtual AstNode* visit(ReturnStatement* node) override;
	virtual AstNode* visit(WhileStatement* node) override;
	virtual AstNode* visit(IfStatement* node) override;

	virtual AstNode* visit(StructDeclaration* node) override;
	virtual AstNode* visit(FunctionDeclaration* node) override;
	virtual AstNode* visit(ExternFunctionDeclaration* node) override;
	virtual AstNode* visit(Module* node) override;

	virtual AstNode* visit(AstNode* node) override { return node->accept(this); }
	virtual AstNode* visit(Expression* node) override { return node->accept(this); }
	virtual AstNode* visit(Statement* node) override { return node->accept(this); }
	virtual AstNode* visit(Declaration* node) override { return node->accept(this); }
	virtual AstNode* visit(BoundCallExpression* node) override { throw std::exception(); }
	virtual AstNode* visit(BoundIndexExpression* node) override { throw std::exception(); }

private:
	template<typename Tr, typename Tv>
	Tr* checked_cast(Tv* value)
	{
		auto ptr = dynamic_cast<Tr*>(value);
		if (!ptr)
			throw std::exception("bad check_cast");
		return ptr;
	}

	void error(AstNode* node, std::string const& message) { return ErrorLogger::error(node->begin, node->end, message); }
	void warning(AstNode* node, std::string const& message) { return ErrorLogger::warning(node->begin, node->end, message); }

	template<typename ReturnType>
	ReturnType error(AstNode* node, std::string const& message, ReturnType&& returnValue)
	{
		error(node, message);
		return std::forward<ReturnType>(returnValue);
	}

	template<typename ReturnType>
	ReturnType warning(AstNode* node, std::string const& message, ReturnType&& returnValue)
	{
		warning(node, message);
		return std::forward<ReturnType>(returnValue);
	}
};