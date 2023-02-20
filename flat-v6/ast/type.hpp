#pragma once
#include <string>
#include <vector>

#include "ast_node.hpp"

struct ASTType : public ASTNode
{
    ASTType(SourceRef const& location) : ASTNode(location) {}

    IMPLEMENT_ACCEPT()
};

struct ASTNamedType : public ASTType
{
    std::string name;
    std::vector<ASTType*> typeArgs;

    ASTNamedType(
        SourceRef const& location,
        std::string const& name,
        std::vector<ASTType*> const& typeArgs
    )
        : ASTType(location), name(name), typeArgs(typeArgs)
    {
    }

    IMPLEMENT_ACCEPT()
};

struct ASTPointerType : public ASTType
{
    ASTType* base;

    ASTPointerType(SourceRef const& location, ASTType* base)
        : ASTType(location), base(base)
    {
    }

    IMPLEMENT_ACCEPT()
};

struct ASTArrayType : public ASTType
{
    ASTType* base;

    ASTArrayType(SourceRef const& location, ASTType* base)
        : ASTType(location), base(base)
    {
    }

    IMPLEMENT_ACCEPT()
};