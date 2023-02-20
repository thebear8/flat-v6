#pragma once
#include "../ast/ast.hpp"
#include "../compiler.hpp"
#include "../ir/ir.hpp"

class StructExtractionPass : ASTVisitor<void>
{
private:
    ErrorLogger& logger;
    CompilationContext& compCtx;
    ModuleContext& modCtx;

public:
    StructExtractionPass(
        ErrorLogger& logger, CompilationContext& compCtx, ModuleContext& modCtx
    )
        : logger(logger), compCtx(compCtx), modCtx(modCtx)
    {
    }

public:
    void process(ASTSourceFile* sourceFile);

private:
    virtual void visit(ASTStructDeclaration* node) override;
    virtual void visit(ASTConstraintDeclaration* node) override {}
    virtual void visit(ASTFunctionDeclaration* node) override {}
    virtual void visit(ASTSourceFile* node) override;
};