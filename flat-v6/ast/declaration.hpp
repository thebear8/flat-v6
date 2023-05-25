#pragma once
#include <string>
#include <vector>

#include "ast_node.hpp"

class IRConstraintTemplate;
class IRStructTemplate;
class IRNormalFunction;

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

struct ASTRequirement : public ASTNode
{
    std::string constraintName;
    std::vector<ASTType*> typeArgs;

    ASTRequirement(
        SourceRef const& location,
        std::string const& constraintName,
        std::vector<ASTType*> const& typeArgs
    )
        : ASTNode(location), constraintName(constraintName), typeArgs(typeArgs)
    {
    }

    IMPLEMENT_ACCEPT()
};

struct ASTConstraintCondition : public ASTNode
{
    std::string functionName;
    std::vector<std::pair<std::string, ASTType*>> params;
    ASTType* result;

    ASTConstraintCondition(
        SourceRef const& location,
        std::string const& functionName,
        std::vector<std::pair<std::string, ASTType*>> const& params,
        ASTType* result
    )
        : ASTNode(location),
          functionName(functionName),
          params(params),
          result(result)
    {
    }

    IMPLEMENT_ACCEPT()
};

struct ASTConstraintDeclaration : public ASTDeclaration
{
    std::string name;
    std::vector<ASTRequirement*> requirements;
    std::vector<ASTConstraintCondition*> conditions;

    ASTConstraintDeclaration(
        SourceRef const& location,
        std::string const& name,
        std::vector<std::string> const& typeParams,
        std::vector<ASTRequirement*> const& requirements,
        std::vector<ASTConstraintCondition*> const& conditions
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
    std::vector<ASTRequirement*> requirements;
    ASTStatement* body;

    ASTFunctionDeclaration(
        SourceRef const& location,
        std::string const& lib,
        std::string const& name,
        std::vector<std::string> const& typeParams,
        std::vector<std::pair<std::string, ASTType*>> const& parameters,
        ASTType* result,
        std::vector<ASTRequirement*> const& requirements,
        ASTStatement* body
    )
        : ASTDeclaration(location, typeParams),
          lib(lib),
          name(name),
          parameters(parameters),
          result(result),
          requirements(requirements),
          body(body)
    {
    }

    IMPLEMENT_ACCEPT()

    METADATA_PROP(irFunction, IRNormalFunction*, getIRFunction, setIRFunction)
};