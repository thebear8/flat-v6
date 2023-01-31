#include "environment.hpp"

#include <algorithm>
#include <ranges>

Type* Environment::getType(std::string const& typeName)
{
    if (auto t = getStruct(typeName))
        return t;
    else if (auto t = getGeneric(typeName))
        return t;

    return nullptr;
}

Type* Environment::findType(std::string const& typeName)
{
    if (auto t = getType(typeName))
        return t;
    else if (m_parent)
        return m_parent->findType(typeName);

    return nullptr;
}

GenericType* Environment::addGeneric(GenericType* genericType)
{
    if (m_generics.contains(genericType->name))
        return nullptr;

    m_generics.try_emplace(genericType->name, genericType);
    return m_generics.at(genericType->name);
}

GenericType* Environment::getGeneric(std::string const& genericName)
{
    if (m_generics.contains(genericName))
        return m_generics.at(genericName);
    return nullptr;
}

GenericType* Environment::findGeneric(std::string const& genericName)
{
    if (auto t = getGeneric(genericName))
        return t;
    if (m_parent)
        return m_parent->findGeneric(genericName);

    return nullptr;
}

StructType* Environment::addStruct(StructType* structType)
{
    if (m_structs.contains(structType->name))
        return nullptr;

    m_structs.try_emplace(structType->name, structType);
    return m_structs.at(structType->name);
}

StructType* Environment::getStruct(std::string const& structName)
{
    if (m_structs.contains(structName))
        return m_structs.at(structName);
    return nullptr;
}

StructType* Environment::findStruct(std::string const& structName)
{
    if (auto structType = getStruct(structName))
        return structType;
    else if (m_parent)
        return m_parent->findStruct(structName);

    return nullptr;
}

IRFunctionDeclaration* Environment::addFunction(IRFunctionDeclaration* function)
{
    for (auto [i, end] = m_functions.equal_range(function->name); i != end; ++i)
    {
        auto t = [](std::pair<std::string, Type*> p)
        {
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

IRFunctionDeclaration* Environment::getFunction(
    std::string const& functionName, std::vector<Type*> const& params)
{
    for (auto [i, end] = m_functions.equal_range(functionName); i != end; ++i)
    {
        auto t = [](std::pair<std::string, Type*> p)
        {
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

IRFunctionDeclaration* Environment::findFunction(
    std::string const& functionName, std::vector<Type*> const& params)
{
    if (auto f = getFunction(functionName, params))
        return f;
    else if (m_parent)
        return m_parent->findFunction(functionName, params);

    return nullptr;
}

Type* Environment::addVariableType(std::string const& variableName, Type* variableType)
{
    if (m_variableTypes.contains(variableName))
        return nullptr;

    m_variableTypes.try_emplace(variableName, variableType);
    return m_variableTypes.at(variableName);
}

Type* Environment::getVariableType(std::string const& variableName)
{
    if (m_variableTypes.contains(variableName))
        return m_variableTypes.at(variableName);
    return nullptr;
}

Type* Environment::findVariableType(std::string const& variableName)
{
    if (auto t = getVariableType(variableName))
        return t;
    else if (m_parent)
        return m_parent->getVariableType(variableName);

    return nullptr;
}