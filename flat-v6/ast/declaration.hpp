#pragma once
#include <string>
#include <vector>

#include "ast_node.hpp"

struct ASTDeclaration : public ASTNode
{
    std::vector<std::string> typeParams;
    std::vector<std::pair<std::string, std::vector<ASTType*>>> requirements;

    ASTDeclaration(
        SourceRef const& location,
        std::vector<std::string> const& typeParams,
        std::vector<std::pair<std::string, std::vector<ASTType*>>> const&
            requirements
    )
        : ASTNode(location), typeParams(typeParams), requirements(requirements)
    {
    }

    IMPLEMENT_ACCEPT()
};

struct ASTConstraintDeclaration : public ASTDeclaration
{
    std::string name;
    std::vector<ASTFunctionDeclaration*> conditions;

    ASTConstraintDeclaration(
        SourceRef const& location,
        std::string const& name,
        std::vector<std::string> const& typeParams,
        std::vector<std::pair<std::string, std::vector<ASTType*>>> const&
            requirements,
        std::vector<ASTFunctionDeclaration*> const& conditions
    )
        : ASTDeclaration(location, typeParams, requirements),
          name(name),
          conditions(conditions)
    {
    }

    IMPLEMENT_ACCEPT()
};

struct ASTStructDeclaration : public ASTDeclaration
{
    std::string name;
    std::vector<std::pair<std::string, ASTType*>> fields;

    ASTStructDeclaration(
        SourceRef const& location,
        std::string const& name,
        std::vector<std::string> const& typeParams,
        std::vector<std::pair<std::string, std::vector<ASTType*>>> const&
            requirements,
        std::vector<std::pair<std::string, ASTType*>> const& fields
    )
        : ASTDeclaration(location, typeParams, requirements),
          name(name),
          fields(fields)
    {
    }

    IMPLEMENT_ACCEPT()
};

struct ASTFunctionDeclaration : public ASTDeclaration
{
    std::string lib, name;
    std::vector<std::pair<std::string, ASTType*>> parameters;
    ASTType* result;
    ASTStatement* body;

    ASTFunctionDeclaration(
        SourceRef const& location,
        std::string const& name,
        std::vector<std::string> const& typeParams,
        std::vector<std::pair<std::string, std::vector<ASTType*>>> const&
            requirements,
        std::vector<std::pair<std::string, ASTType*>> const& parameters,
        ASTType* result,
        ASTStatement* body
    )
        : ASTDeclaration(location, typeParams, requirements),
          name(name),
          parameters(parameters),
          result(result),
          body(body)
    {
    }

    ASTFunctionDeclaration(
        SourceRef const& location,
        std::string const& lib,
        std::string const& name,
        std::vector<std::string> const& typeParams,
        std::vector<std::pair<std::string, std::vector<ASTType*>>> const&
            requirements,
        std::vector<std::pair<std::string, ASTType*>> const& parameters,
        ASTType* result
    )
        : ASTDeclaration(location, typeParams, requirements),
          lib(lib),
          name(name),
          parameters(parameters),
          result(result),
          body(nullptr)
    {
    }

    IMPLEMENT_ACCEPT()
};