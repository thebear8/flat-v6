#include "environment.hpp"

#include <algorithm>
#include <ranges>

IRType* Environment::getType(std::string const& typeName)
{
    if (auto s = getStruct(typeName))
        return s;
    else if (auto g = getGeneric(typeName))
        return g;

    return nullptr;
}

IRType* Environment::findType(std::string const& typeName)
{
    if (auto t = getType(typeName))
        return t;
    else if (m_parent)
        return m_parent->findType(typeName);

    return nullptr;
}

IRGenericType* Environment::addGeneric(IRGenericType* genericType)
{
    if (m_generics.contains(genericType->name))
        return nullptr;

    m_generics.try_emplace(genericType->name, genericType);
    return m_generics.at(genericType->name);
}

IRGenericType* Environment::getGeneric(std::string const& genericName)
{
    if (m_generics.contains(genericName))
        return m_generics.at(genericName);
    return nullptr;
}

IRGenericType* Environment::findGeneric(std::string const& genericName)
{
    if (auto t = getGeneric(genericName))
        return t;
    if (m_parent)
        return m_parent->findGeneric(genericName);

    return nullptr;
}

IRConstraint* Environment::addConstraint(IRConstraint* constraint)
{
    if (m_constraints.contains(constraint->name))
        return nullptr;

    m_constraints.try_emplace(constraint->name, constraint);
    return m_constraints.at(constraint->name);
}

IRConstraint* Environment::getConstraint(std::string const& constraintName)
{
    if (m_constraints.contains(constraintName))
        return m_constraints.at(constraintName);
    return nullptr;
}

IRConstraint* Environment::findConstraint(std::string const& constraintName)
{
    if (auto constraint = getConstraint(constraintName))
        return constraint;
    else if (m_parent)
        return m_parent->findConstraint(constraintName);

    return nullptr;
}

IRStructType* Environment::addStruct(IRStructType* structType)
{
    if (m_structs.contains(structType->name))
        return nullptr;

    m_structs.try_emplace(structType->name, structType);
    return m_structs.at(structType->name);
}

IRStructType* Environment::getStruct(std::string const& structName)
{
    if (m_structs.contains(structName))
        return m_structs.at(structName);
    return nullptr;
}

IRStructType* Environment::findStruct(std::string const& structName)
{
    if (auto structType = getStruct(structName))
        return structType;
    else if (m_parent)
        return m_parent->findStruct(structName);

    return nullptr;
}

IRFunction* Environment::addFunction(IRFunction* function)
{
    for (auto [i, end] = m_functions.equal_range(function->name); i != end; ++i)
    {
        auto t = [](std::pair<std::string, IRType*> p) {
            return p.second;
        };

        auto candidate = i->second;
        auto functionParams = function->params | std::views::transform(t);
        auto candidateParams = candidate->params | std::views::transform(t);

        if (functionParams.size() == candidateParams.size()
            && std::ranges::equal(functionParams, candidateParams))
            return nullptr;
    }

    m_functions.insert(std::pair(function->name, function));
    return function;
}

IRFunction* Environment::getFunction(
    std::string const& functionName, std::vector<IRType*> const& params
)
{
    for (auto [i, end] = m_functions.equal_range(functionName); i != end; ++i)
    {
        auto t = [](std::pair<std::string, IRType*> p) {
            return p.second;
        };

        auto candidate = i->second;
        auto candidateParams = candidate->params | std::views::transform(t);

        if (params.size() == candidateParams.size()
            && std::ranges::equal(params, candidateParams))
            return candidate;
    }

    return nullptr;
}

IRFunction* Environment::findFunction(
    std::string const& functionName, std::vector<IRType*> const& params
)
{
    if (auto f = getFunction(functionName, params))
        return f;
    else if (m_parent)
        return m_parent->findFunction(functionName, params);

    return nullptr;
}

IRType* Environment::addVariableType(
    std::string const& variableName, IRType* variableType
)
{
    if (m_variableTypes.contains(variableName))
        return nullptr;

    m_variableTypes.try_emplace(variableName, variableType);
    return m_variableTypes.at(variableName);
}

IRType* Environment::getVariableType(std::string const& variableName)
{
    if (m_variableTypes.contains(variableName))
        return m_variableTypes.at(variableName);
    return nullptr;
}

IRType* Environment::findVariableType(std::string const& variableName)
{
    if (auto t = getVariableType(variableName))
        return t;
    else if (m_parent)
        return m_parent->getVariableType(variableName);

    return nullptr;
}

bool Environment::inferTypeArgsAndMatch(
    IRType* genericType,
    IRType* actualType,
    std::unordered_map<IRGenericType*, IRType*>& typeArgs
)
{
    if (actualType == genericType)
    {
        return true;
    }
    else if (genericType->isGenericType())
    {
        if (!typeArgs.contains((IRGenericType*)genericType))
            typeArgs.try_emplace((IRGenericType*)genericType, actualType);

        if (typeArgs.at((IRGenericType*)genericType) != actualType)
            return false;

        return true;
    }
    else if (genericType->isStructInstantiation())
    {
        if (!actualType->isStructInstantiation())
            return false;

        auto genericInstantiated = (IRInstantiatedStructType*)genericType;
        auto actualInstantiated = (IRInstantiatedStructType*)actualType;

        if (actualInstantiated->base != genericInstantiated->base)
            return false;

        if (actualInstantiated->typeArgs.size()
            != genericInstantiated->typeArgs.size())
            return false;

        for (size_t i = 0; i < genericInstantiated->typeArgs.size(); i++)
        {
            auto result = inferTypeArgsAndMatch(
                genericInstantiated->typeArgs.at(i),
                actualInstantiated->typeArgs.at(i),
                typeArgs
            );

            if (!result)
                return false;
        }

        return true;
    }
    else if (genericType->isPointerType())
    {
        if (!actualType->isPointerType())
            return false;

        return inferTypeArgsAndMatch(
            ((IRPointerType*)genericType)->base,
            ((IRPointerType*)actualType)->base,
            typeArgs
        );
    }
    else if (genericType->isArrayType())
    {
        if (!actualType->isArrayType())
            return false;

        return inferTypeArgsAndMatch(
            ((IRArrayType*)genericType)->base,
            ((IRArrayType*)actualType)->base,
            typeArgs
        );
    }
    else
    {
        return false;
    }
}

std::optional<std::string> Environment::inferTypeArgsAndValidate(
    IRType* genericType,
    IRType* actualType,
    std::unordered_map<IRGenericType*, IRType*>& typeArgs
)
{
    if (actualType == genericType)
    {
        return std::nullopt;
    }
    else if (genericType->isGenericType())
    {
        if (!typeArgs.contains((IRGenericType*)genericType))
            typeArgs.try_emplace((IRGenericType*)genericType, actualType);

        if (typeArgs.at((IRGenericType*)genericType) != actualType)
        {
            return "Inconsistent value for type parameter "
                + genericType->toString() + ": was "
                + typeArgs.at((IRGenericType*)genericType)->toString()
                + ", now " + actualType->toString();
        }

        return std::nullopt;
    }
    else if (genericType->isStructInstantiation())
    {
        if (!actualType->isStructInstantiation())
        {
            return "Type " + actualType->toString()
                + " is not an instantiated struct type";
        }

        auto genericInstantiated = (IRInstantiatedStructType*)genericType;
        auto actualInstantiated = (IRInstantiatedStructType*)actualType;

        if (actualInstantiated->base != genericInstantiated->base)
        {
            return "Type " + actualInstantiated->toString()
                + " is not an instantiation of "
                + genericInstantiated->toString();
        }

        if (actualInstantiated->typeArgs.size()
            != genericInstantiated->typeArgs.size())
        {
            return "Number of type args of " + actualInstantiated->toString()
                + " does not match number of type args of "
                + genericInstantiated->toString();
        }

        for (size_t i = 0; i < genericInstantiated->typeArgs.size(); i++)
        {
            auto result = inferTypeArgsAndValidate(
                genericInstantiated->typeArgs.at(i),
                actualInstantiated->typeArgs.at(i),
                typeArgs
            );

            if (result.has_value())
                return result;
        }

        return std::nullopt;
    }
    else if (genericType->isPointerType())
    {
        if (!actualType->isPointerType())
        {
            return "Type " + actualType->toString() + " is not a pointer type";
        }

        return inferTypeArgsAndValidate(
            ((IRPointerType*)genericType)->base,
            ((IRPointerType*)actualType)->base,
            typeArgs
        );
    }
    else if (genericType->isArrayType())
    {
        if (!actualType->isArrayType())
        {
            return "Type " + actualType->toString() + " is not an array type";
        }

        return inferTypeArgsAndValidate(
            ((IRArrayType*)genericType)->base,
            ((IRArrayType*)actualType)->base,
            typeArgs
        );
    }
    else
    {
        return "Types " + genericType->toString() + " and "
            + actualType->toString() + " do not match";
    }
}