#include "environment.hpp"

#include <algorithm>
#include <ranges>

#include "util/zip_view.hpp"

IRType* Environment::addBuiltinType(
    std::string const& name, IRType* builtinType
)
{
    if (m_builtinTypes.contains(name))
        return nullptr;

    m_builtinTypes.try_emplace(name, builtinType);
    return m_builtinTypes.at(name);
}

IRType* Environment::getBuiltinType(std::string const& name)
{
    if (!m_builtinTypes.contains(name))
        return nullptr;
    return m_builtinTypes.at(name);
}

IRType* Environment::findBuiltinType(std::string const& name)
{
    if (auto b = getBuiltinType(name))
        return b;
    else if (m_parent)
        return m_parent->findBuiltinType(name);
    return nullptr;
}

IRGenericType* Environment::addTypeParam(IRGenericType* typeParam)
{
    if (m_typeParams.contains(typeParam->name))
        return nullptr;

    m_typeParams.try_emplace(typeParam->name, typeParam);
    return m_typeParams.at(typeParam->name);
}

IRGenericType* Environment::getTypeParam(std::string const& name)
{
    if (m_typeParams.contains(name))
        return m_typeParams.at(name);
    return nullptr;
}

IRGenericType* Environment::findTypeParam(std::string const& name)
{
    if (auto t = getTypeParam(name))
        return t;
    if (m_parent)
        return m_parent->findTypeParam(name);

    return nullptr;
}

IRType* Environment::addTypeParamValue(IRGenericType* typeParam, IRType* value)
{
    if (m_typeParamValues.contains(typeParam))
        return nullptr;

    m_typeParamValues.try_emplace(typeParam, value);
    return m_typeParamValues.at(typeParam);
}

IRType* Environment::getTypeParamValue(IRGenericType* typeParam)
{
    if (!m_typeParamValues.contains(typeParam))
        return nullptr;
    return m_typeParamValues.at(typeParam);
}

IRType* Environment::findTypeParamValue(IRGenericType* typeParam)
{
    if (auto v = getTypeParamValue(typeParam))
        return v;
    else if (m_parent)
        return m_parent->findTypeParamValue(typeParam);

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

IRStructTemplate* Environment::addStruct(IRStructTemplate* structType)
{
    if (m_structs.contains(structType->name))
        return nullptr;

    m_structs.try_emplace(structType->name, structType);
    return m_structs.at(structType->name);
}

IRStructTemplate* Environment::getStruct(std::string const& structName)
{
    if (m_structs.contains(structName))
        return m_structs.at(structName);
    return nullptr;
}

IRStructTemplate* Environment::findStruct(std::string const& structName)
{
    if (auto structType = getStruct(structName))
        return structType;
    else if (m_parent)
        return m_parent->findStruct(structName);

    return nullptr;
}

IRFunctionTemplate* Environment::addFunction(IRFunctionTemplate* function)
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

IRFunctionTemplate* Environment::getFunction(
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

IRFunctionTemplate* Environment::findFunction(
    std::string const& functionName, std::vector<IRType*> const& params
)
{
    if (auto f = getFunction(functionName, params))
        return f;
    else if (m_parent)
        return m_parent->findFunction(functionName, params);

    return nullptr;
}

IRFunctionTemplate* Environment::findCallTargetAndInferTypeArgs(
    std::string const& name,
    std::vector<IRType*> const& args,
    std::unordered_map<IRGenericType*, IRType*>& typeArgs
)
{
    for (auto [i, end] = m_functions.equal_range(name); i != end; ++i)
    {
        auto candidate = i->second;
        auto candidateParams = candidate->params | std::views::values;

        if (args.size() != candidateParams.size())
            continue;

        typeArgs.clear();

        auto incompatibleParams =
            zip_view(args, candidateParams)
            | std::views::filter([&](auto const& p) {
                  return !inferTypeArgsAndMatch(p.first, p.second, typeArgs);
              });

        if (incompatibleParams.empty())
            return candidate;
    }
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
        return m_parent->findVariableType(variableName);

    return nullptr;
}

llvm::Value* Environment::setVariableValue(
    std::string const& name, llvm::Value* value
)
{
    m_llvmVariableValues[name] = value;
    return m_llvmVariableValues.at(name);
}

llvm::Value* Environment::getVariableValue(std::string const& name)
{
    if (!m_llvmVariableValues.contains(name))
        return nullptr;
    return m_llvmVariableValues.at(name);
}

llvm::Value* Environment::findVariableValue(std::string const& name)
{
    if (auto v = getVariableValue(name))
        return v;
    else if (m_parent)
        return m_parent->findVariableValue(name);

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

        auto genericInstantiated = (IRStructInstantiation*)genericType;
        auto actualInstantiated = (IRStructInstantiation*)actualType;

        if (actualInstantiated->getInstantiatedFrom()
            != genericInstantiated->getInstantiatedFrom())
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

        auto genericInstantiated = (IRStructInstantiation*)genericType;
        auto actualInstantiated = (IRStructInstantiation*)actualType;

        if (actualInstantiated->getInstantiatedFrom()
            != genericInstantiated->getInstantiatedFrom())
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