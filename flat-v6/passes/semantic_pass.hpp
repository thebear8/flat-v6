#pragma once
#include <string>
#include <string_view>
#include <ostream>
#include <unordered_map>

#include "../data/ir.hpp"
#include "../type/type.hpp"
#include "../util/error_logger.hpp"
#include "../data/operator.hpp"
#include "../compiler.hpp"

class SemanticPass : protected IRVisitor<Type*>
{
private:
	ErrorLogger& logger;
	CompilationContext& compCtx;
	ModuleContext& modCtx;

	Type* functionResult, * expectedFunctionResult;
	std::unordered_map<std::string, ASTStructDeclaration*> structs;
	std::unordered_multimap<std::string, ASTFunctionDeclaration*> functions;
	std::unordered_multimap<std::string, ASTExternFunctionDeclaration*> externFunctions;
	std::unordered_map<std::string, Type*> localVariables;

public:
	SemanticPass(ErrorLogger& logger, CompilationContext& compCtx, ModuleContext& modCtx) :
		logger(logger), compCtx(compCtx), modCtx(modCtx), functionResult(nullptr), expectedFunctionResult(nullptr) { }

public:
	void analyze(IRSourceFile* source);

protected:
	Type* getFunctionResult(std::string const& name, std::vector<Type*> const& args);
	Type* getFunctionResult(std::string const& name, std::vector<Type*> const& args, ASTNode* current);

protected:
	virtual Type* visit(IRIntegerExpression* node) override;
	virtual Type* visit(IRBoolExpression* node) override;
	virtual Type* visit(IRCharExpression* node) override;
	virtual Type* visit(IRStringExpression* node) override;
	virtual Type* visit(IRIdentifierExpression* node) override;
	virtual Type* visit(IRStructExpression* node) override;
	virtual Type* visit(IRUnaryExpression* node) override;
	virtual Type* visit(IRBinaryExpression* node) override;
	virtual Type* visit(IRCallExpression* node) override;
	virtual Type* visit(IRIndexExpression* node) override;
	virtual Type* visit(IRFieldExpression* node) override;

	virtual Type* visit(IRBlockStatement* node) override;
	virtual Type* visit(IRExpressionStatement* node) override;
	virtual Type* visit(IRVariableStatement* node) override;
	virtual Type* visit(IRReturnStatement* node) override;
	virtual Type* visit(IRWhileStatement* node) override;
	virtual Type* visit(IRIfStatement* node) override;

	virtual Type* visit(IRStructDeclaration* node) override;
	virtual Type* visit(IRFunctionDeclaration* node) override;
	virtual Type* visit(IRSourceFile* node) override;
};