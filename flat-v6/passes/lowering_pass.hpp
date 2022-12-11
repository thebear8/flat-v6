#pragma once
#include <string>
#include <string_view>
#include <ostream>

#include "../data/ast.hpp"
#include "../util/error_logger.hpp"
#include "../data/operator.hpp"

class OperatorLoweringPass : protected Visitor<ASTNode*>
{
private:
	ErrorLogger& logger;
	AstContext& astCtx;
	TypeContext& typeCtx;

public:
	OperatorLoweringPass(ErrorLogger& logger, AstContext& astCtx, TypeContext& ctx) :
		logger(logger), astCtx(astCtx), typeCtx(ctx) { }

public:
	ASTNode* process(ASTNode* program);

protected:
	virtual ASTNode* visit(ASTIntegerExpression* node) override;
	virtual ASTNode* visit(ASTBoolExpression* node) override;
	virtual ASTNode* visit(ASTCharExpression* node) override;
	virtual ASTNode* visit(ASTStringExpression* node) override;
	virtual ASTNode* visit(ASTIdentifierExpression* node) override;
	virtual ASTNode* visit(ASTStructExpression* node) override;
	virtual ASTNode* visit(ASTUnaryExpression* node) override;
	virtual ASTNode* visit(ASTBinaryExpression* node) override;
	virtual ASTNode* visit(ASTCallExpression* node) override;
	virtual ASTNode* visit(ASTIndexExpression* node) override;
	virtual ASTNode* visit(ASTFieldExpression* node) override;

	virtual ASTNode* visit(ASTBlockStatement* node) override;
	virtual ASTNode* visit(ASTExpressionStatement* node) override;
	virtual ASTNode* visit(ASTVariableStatement* node) override;
	virtual ASTNode* visit(ASTReturnStatement* node) override;
	virtual ASTNode* visit(ASTWhileStatement* node) override;
	virtual ASTNode* visit(ASTIfStatement* node) override;

	virtual ASTNode* visit(ASTStructDeclaration* node) override;
	virtual ASTNode* visit(ASTFunctionDeclaration* node) override;
	virtual ASTNode* visit(ASTExternFunctionDeclaration* node) override;
	virtual ASTNode* visit(ASTSourceFile* node) override;

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