#pragma once
#include <unordered_map>
#include <vector>

#include "../../ast/ast.hpp"

class ErrorLogger;
class CompilationContext;
class GraphContext;
class ASTTypeResolver;
class Instantiator;
class Environment;
class IRNode;
class IRModule;

class FunctionPopulationPass : ASTVisitor<IRNode*>
{
private:
    ErrorLogger& m_logger;
    CompilationContext& m_compCtx;
    GraphContext& m_envCtx;
    ASTTypeResolver& m_resolver;
    Instantiator& m_instantiator;

    IRModule* m_module = nullptr;
    GraphContext* m_irCtx = nullptr;
    Environment* m_env = nullptr;

public:
    FunctionPopulationPass(
        ErrorLogger& logger,
        CompilationContext& compCtx,
        GraphContext& envCtx,
        ASTTypeResolver& resolver,
        Instantiator& instantiator
    )
        : m_logger(logger),
          m_compCtx(compCtx),
          m_envCtx(envCtx),
          m_resolver(resolver),
          m_instantiator(instantiator)
    {
    }

public:
    IRModule* process(ASTSourceFile* sourceFile);

private:
    IRNode* visit(ASTIntegerExpression* node) override;
    IRNode* visit(ASTBoolExpression* node) override;
    IRNode* visit(ASTCharExpression* node) override;
    IRNode* visit(ASTStringExpression* node) override;
    IRNode* visit(ASTIdentifierExpression* node) override;
    IRNode* visit(ASTStructExpression* node) override;
    IRNode* visit(ASTUnaryExpression* node) override;
    IRNode* visit(ASTBinaryExpression* node) override;
    IRNode* visit(ASTCallExpression* node) override;
    IRNode* visit(ASTIndexExpression* node) override;
    IRNode* visit(ASTFieldExpression* node) override;

    IRNode* visit(ASTBlockStatement* node) override;
    IRNode* visit(ASTExpressionStatement* node) override;
    IRNode* visit(ASTVariableStatement* node) override;
    IRNode* visit(ASTReturnStatement* node) override;
    IRNode* visit(ASTWhileStatement* node) override;
    IRNode* visit(ASTIfStatement* node) override;

    IRNode* visit(ASTRequirement* node) override;
    IRNode* visit(ASTConstraintDeclaration* node) override { return nullptr; }
    IRNode* visit(ASTStructDeclaration* node) override { return nullptr; }
    IRNode* visit(ASTFunctionDeclaration* node) override;
    IRNode* visit(ASTSourceFile* node) override;

private:
    std::vector<uint8_t> unescapeStringUTF8(
        std::string const& input, SourceRef const& location
    );
    uint32_t unescapeCodePoint(
        std::string const& value,
        std::size_t& position,
        SourceRef const& location
    );

    bool isDigit(char c) { return (c >= '0' && c <= '9'); }
    bool isBinaryDigit(char c) { return (c >= '0' && c <= '1'); }
    bool isHexDigit(char c)
    {
        return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f')
            || (c >= 'A' && c <= 'F');
    }
    bool isLetter(char c)
    {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
    }
    bool isWhitespace(char c)
    {
        return (c == ' ' || c == '\t' || c == '\r' || c == '\n');
    }
    bool isIdentifier(char c)
    {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')
            || (c >= '0' && c <= '9') || (c == '_');
    }

private:
    static inline std::unordered_map<char, uint32_t> m_escapeChars = {
        { 'a', '\a' },  { 'b', '\b' },  { 'f', '\f' },  { 'n', '\n' },
        { 'r', '\r' },  { 't', '\t' },  { 'v', '\v' },  { '\\', '\\' },
        { '\'', '\'' }, { '\"', '\"' }, { '\?', '\?' },
    };
};