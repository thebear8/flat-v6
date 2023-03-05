#pragma once
#include <string>
#include <vector>

#include "ast_node.hpp"

class IRConstraintTemplate;
class IRStructTemplate;
class IRFunctionTemplate;

struct ASTDeclaration : public ASTNode
{
    std::vector<std::string> typeParams;

    ASTDeclaration(
        SourceRef const& location, std::vector<std::string> const& typeParams
    )
        : ASTNode(location), typeParams(typeParams)
    {
    }

    IMPLEMENT_ACCEPT()
};

struct ASTConstraintDeclaration : public ASTDeclaration
{
    std::string name;
    std::vector<std::pair<std::string, std::vector<ASTType*>>> requirements;
    std::vector<ASTFunctionDeclaration*> conditions;

    ASTConstraintDeclaration(
        SourceRef const& location,
        std::string const& name,
        std::vector<std::string> const& typeParams,
        std::vector<std::pair<std::string, std::vector<ASTType*>>> const&
            requirements,
        std::vector<ASTFunctionDeclaration*> const& conditions
    )
        : ASTDeclaration(location, typeParams),
          name(name),
          requirements(requirements),
          conditions(conditions)
    {
    }

    IMPLEMENT_ACCEPT()

    METADATA_PROP(
        irConstraint, IRConstraintTemplate*, getIRConstraint, setIRConstraint
    )
};

struct ASTStructDeclaration : public ASTDeclaration
{
    std::string name;
    std::vector<std::pair<std::string, ASTType*>> fields;

    ASTStructDeclaration(
        SourceRef const& location,
        std::string const& name,
        std::vector<std::string> const& typeParams,
        std::vector<std::pair<std::string, ASTType*>> const& fields
    )
        : ASTDeclaration(location, typeParams), name(name), fields(fields)
    {
    }

    IMPLEMENT_ACCEPT()

    METADATA_PROP(irStruct, IRStructTemplate*, getIRStruct, setIRStruct)
};

struct ASTFunctionDeclaration : public ASTDeclaration
{
    std::string lib, name;
    std::vector<std::pair<std::string, ASTType*>> parameters;
    ASTType* result;
    std::vector<std::pair<std::string, std::vector<ASTType*>>> requirements;
    ASTStatement* body;

    ASTFunctionDeclaration(
        SourceRef const& location,
        std::string const& name,
        std::vector<std::string> const& typeParams,
        std::vector<std::pair<std::string, ASTType*>> const& parameters,
        ASTType* result,
        std::vector<std::pair<std::string, std::vector<ASTType*>>> const&
            requirements,
        ASTStatement* body
    )
        : ASTDeclaration(location, typeParams),
          name(name),
          parameters(parameters),
          result(result),
          requirements(requirements),
          body(body)
    {
    }

    ASTFunctionDeclaration(
        SourceRef const& location,
        std::string const& lib,
        std::string const& name,
        std::vector<std::pair<std::string, ASTType*>> const& parameters,
        ASTType* result
    )
        : ASTDeclaration(location, typeParams),
          lib(lib),
          name(name),
          parameters(parameters),
          result(result),
          body(nullptr)
    {
    }

    IMPLEMENT_ACCEPT()

    METADATA_PROP(irFunction, IRFunctionTemplate*, getIRFunction, setIRFunction)
};