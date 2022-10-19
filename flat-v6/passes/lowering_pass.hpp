#pragma once
#include <string>
#include <string_view>
#include <ostream>

#include "../data/ast.hpp"
#include "../util/error_logger.hpp"
#include "../data/operator.hpp"

class OperatorLoweringPass : protected Visitor<AstNode*>
{
private:
	ErrorLogger& logger;
	AstContext& astCtx;
	TypeContext& typeCtx;

public:
	OperatorLoweringPass(ErrorLogger& logger, AstContext& astCtx, TypeContext& ctx) :
		logger(logger), astCtx(astCtx), typeCtx(ctx) { }

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
	virtual AstNode* visit(ParsedSourceFile* node) override;

private:
	template<typename Tr, typename Tv>
	Tr* checked_cast(Tv* value)
	{
		auto ptr = dynamic_cast<Tr*>(value);
		if (!ptr)
			throw std::exception("bad check_cast");
		return ptr;
	}
};