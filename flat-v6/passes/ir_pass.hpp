#pragma once
#include "../compiler.hpp"
#include "../data/ast.hpp"
#include "../data/ir.hpp"
#include "../data/source_ref.hpp"

/// @brief Transforms AST Nodes into IR Nodes
class IRPass : public ASTVisitor<IRNode*>
{
private:
    ErrorLogger& m_logger;
    CompilationContext& m_compCtx;
    ModuleContext& m_modCtx;
    GraphContext& m_irCtx;

    GraphContext m_envCtx;
    GraphContext m_genericCtx;

    Environment* m_env;

public:
    IRPass(
        ErrorLogger& logger,
        CompilationContext& compCtx,
        ModuleContext& modCtx,
        GraphContext& irCtx)
        : m_logger(logger),
          m_compCtx(compCtx),
          m_modCtx(modCtx),
          m_irCtx(irCtx),
          m_env(nullptr)
    {
    }

public:
    IRSourceFile* process(ASTSourceFile* sourceFile);

private:
    virtual IRNode* visit(ASTIntegerExpression* node) override;
    virtual IRNode* visit(ASTBoolExpression* node) override;
    virtual IRNode* visit(ASTCharExpression* node) override;
    virtual IRNode* visit(ASTStringExpression* node) override;
    virtual IRNode* visit(ASTIdentifierExpression* node) override;
    virtual IRNode* visit(ASTStructExpression* node) override;
    virtual IRNode* visit(ASTUnaryExpression* node) override;
    virtual IRNode* visit(ASTBinaryExpression* node) override;
    virtual IRNode* visit(ASTCallExpression* node) override;
    virtual IRNode* visit(ASTIndexExpression* node) override;
    virtual IRNode* visit(ASTFieldExpression* node) override;

    virtual IRNode* visit(ASTBlockStatement* node) override;
    virtual IRNode* visit(ASTExpressionStatement* node) override;
    virtual IRNode* visit(ASTVariableStatement* node) override;
    virtual IRNode* visit(ASTReturnStatement* node) override;
    virtual IRNode* visit(ASTWhileStatement* node) override;
    virtual IRNode* visit(ASTIfStatement* node) override;

    virtual IRNode* visit(ASTConstraintDeclaration* node) override;
    virtual IRNode* visit(ASTStructDeclaration* node) override;
    virtual IRNode* visit(ASTFunctionDeclaration* node) override;
    virtual IRNode* visit(ASTExternFunctionDeclaration* node) override;
    virtual IRNode* visit(ASTSourceFile* node) override;

    virtual IRNode* visit(ASTNamedType* node) override;
    virtual IRNode* visit(ASTPointerType* node) override;
    virtual IRNode* visit(ASTArrayType* node) override;

private:
    std::vector<std::pair<std::string, std::vector<IRType*>>>
    transformRequirements(
        std::vector<std::pair<std::string, std::vector<ASTType*>>> const&
            requirements);

private:
    std::vector<uint8_t> unescapeStringUTF8(
        std::string const& input, SourceRef const& location);
    uint32_t unescapeCodePoint(
        std::string const& value, size_t& position, SourceRef const& location);

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
    static inline std::unordered_map<char, uint32_t> escapeChars = {
        { 'a', '\a' },  { 'b', '\b' },  { 'f', '\f' },  { 'n', '\n' },
        { 'r', '\r' },  { 't', '\t' },  { 'v', '\v' },  { '\\', '\\' },
        { '\'', '\'' }, { '\"', '\"' }, { '\?', '\?' },
    };
};