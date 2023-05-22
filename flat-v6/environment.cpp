#include "environment.hpp"

#include <algorithm>
#include <optional>
#include <ranges>

#include "ir/ir.hpp"
#include "util/assert.hpp"
#include "util/to_vector.hpp"
#include "util/zip_view.hpp"

IRType* Environment::addBuiltinType(
    std::string const& name, IRType* builtinType
)
{
    if (getBuiltinType(name))
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
    if (getTypeParam(typeParam->name))
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
    if (getTypeParamValue(typeParam))
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

IRConstraintTemplate* Environment::addConstraint(
    IRConstraintTemplate* constraint
)
{
    if (getConstraint(constraint->name))
        return nullptr;

    m_constraints.try_emplace(constraint->name, constraint);
    return m_constraints.at(constraint->name);
}

IRConstraintTemplate* Environment::getConstraint(
    std::string const& constraintName
)
{
    if (m_constraints.contains(constraintName))
        return m_constraints.at(constraintName);
    return nullptr;
}

IRConstraintTemplate* Environment::findConstraint(
    std::string const& constraintName
)
{
    if (auto constraint = getConstraint(constraintName))
        return constraint;
    else if (m_parent)
        return m_parent->findConstraint(constraintName);

    return nullptr;
}

IRConstraintInstantiation* Environment::addConstraintInstantiation(
    IRConstraintTemplate* constraintTemplate,
    IRConstraintInstantiation* constraintInstantiation
)
{
    if (getConstraintInstantiation(
            constraintTemplate, constraintInstantiation->typeArgs
        ))
    {
        return nullptr;
    }

    m_constraintInstantiations.emplace(
        constraintTemplate, constraintInstantiation
    );
    return constraintInstantiation;
}

IRConstraintInstantiation* Environment::getConstraintInstantiation(
    IRConstraintTemplate* constraintTemplate,
    std::vector<IRType*> const& typeArgs
)
{
    for (auto [it, end] =
             m_constraintInstantiations.equal_range(constraintTemplate);
         it != end;
         ++it)
    {
        if (std::ranges::equal(it->second->typeArgs, typeArgs))
            return it->second;
    }

    return nullptr;
}

IRConstraintInstantiation* Environment::findConstraintInstantiation(
    IRConstraintTemplate* constraintTemplate,
    std::vector<IRType*> const& typeArgs
)
{
    if (auto i = getConstraintInstantiation(constraintTemplate, typeArgs))
    {
        return i;
    }
    else if (m_parent)
    {
        return m_parent->findConstraintInstantiation(
            constraintTemplate, typeArgs
        );
    }

    return nullptr;
}

IRStructTemplate* Environment::addStruct(IRStructTemplate* structType)
{
    if (getStruct(structType->name))
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

IRStructInstantiation* Environment::addStructInstantiation(
    IRStructTemplate* structTemplate, IRStructInstantiation* structInstantiation
)
{
    if (getStructInstantiation(structTemplate, structInstantiation->typeArgs))
        return nullptr;

    m_structInstantiations.emplace(structTemplate, structInstantiation);
    return structInstantiation;
}

IRStructInstantiation* Environment::getStructInstantiation(
    IRStructTemplate* structTemplate, std::vector<IRType*> const& typeArgs
)
{
    for (auto [it, end] = m_structInstantiations.equal_range(structTemplate);
         it != end;
         ++it)
    {
        if (std::ranges::equal(it->second->typeArgs, typeArgs))
            return it->second;
    }

    return nullptr;
}

IRStructInstantiation* Environment::findStructInstantiation(
    IRStructTemplate* structTemplate, std::vector<IRType*> const& typeArgs
)
{
    if (auto i = getStructInstantiation(structTemplate, typeArgs))
        return i;
    else if (m_parent)
        return m_parent->findStructInstantiation(structTemplate, typeArgs);

    return nullptr;
}

IRFunction* Environment::addFunction(IRFunction* function)
{
    m_functions.emplace(function->name, function);
    return function;
}

IRFunction* Environment::addFunctionInstantiation(
    IRFunction* function, IRFunction* instantiation
)
{
    if (getFunctionInstantiation(function, instantiation->typeArgs))
        return nullptr;

    m_functionInstantiations.emplace(function, instantiation);
    return instantiation;
}

IRFunction* Environment::getFunctionInstantiation(
    IRFunction* function, std::vector<IRType*> const& typeArgs
)
{
    for (auto [it, end] = m_functionInstantiations.equal_range(function);
         it != end;
         ++it)
    {
        if (std::ranges::equal(it->second->typeArgs, typeArgs))
            return it->second;
    }

    return nullptr;
}

IRFunction* Environment::findFunctionInstantiation(
    IRFunction* function, std::vector<IRType*> const& typeArgs
)
{
    if (auto i = getFunctionInstantiation(function, typeArgs))
        return i;
    else if (m_parent)
        return m_parent->findFunctionInstantiation(function, typeArgs);

    return nullptr;
}

IRType* Environment::addVariableType(
    std::string const& variableName, IRType* variableType
)
{
    if (getVariableType(variableName))
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
    return (m_llvmVariableValues[name] = value);
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
    std::unordered_map<IRGenericType*, IRType*>& typeArgs,
    bool allowGenericSubstitution,
    optional_ref<std::string> reason
)
{
    if (actualType == genericType)
    {
        return true;
    }
    else if (genericType->isGenericType() && (!actualType->isGenericType() || allowGenericSubstitution))
    {
        if (!typeArgs.contains((IRGenericType*)genericType))
            typeArgs.try_emplace((IRGenericType*)genericType, actualType);

        if (typeArgs.at((IRGenericType*)genericType) != actualType)
        {
            if (reason.has_value())
            {
                reason = "Inconsistent value for type parameter "
                    + genericType->toString() + ": was "
                    + typeArgs.at((IRGenericType*)genericType)->toString()
                    + ", now " + actualType->toString();
            }

            return false;
        }

        return true;
    }
    else if (genericType->isStructInstantiation())
    {
        if (!actualType->isStructInstantiation())
        {
            if (reason.has_value())
            {
                reason = "Type " + actualType->toString()
                    + " is not an instantiated struct type";
            }

            return false;
        }

        auto genericInstantiated = (IRStructInstantiation*)genericType;
        auto actualInstantiated = (IRStructInstantiation*)actualType;

        if (actualInstantiated->getInstantiatedFrom()
            != genericInstantiated->getInstantiatedFrom())
        {
            if (reason.has_value())
            {
                reason = "Type " + actualInstantiated->toString()
                    + " is not an instantiation of "
                    + genericInstantiated->toString();
            }

            return false;
        }

        if (actualInstantiated->typeArgs.size()
            != genericInstantiated->typeArgs.size())
        {
            if (reason.has_value())
            {
                reason = "Number of type args of "
                    + actualInstantiated->toString()
                    + " does not match number of type args of "
                    + genericInstantiated->toString();
            }

            return false;
        }

        for (std::size_t i = 0; i < genericInstantiated->typeArgs.size(); i++)
        {
            if (!inferTypeArgsAndMatch(
                    genericInstantiated->typeArgs.at(i),
                    actualInstantiated->typeArgs.at(i),
                    typeArgs,
                    allowGenericSubstitution,
                    reason
                ))
            {
                return false;
            }
        }

        return true;
    }
    else if (genericType->isPointerType())
    {
        if (!actualType->isPointerType())
        {
            if (reason.has_value())
            {
                reason =
                    "Type " + actualType->toString() + " is not a pointer type";
            }

            return false;
        }

        return inferTypeArgsAndMatch(
            ((IRPointerType*)genericType)->base,
            ((IRPointerType*)actualType)->base,
            typeArgs,
            allowGenericSubstitution,
            reason
        );
    }
    else if (genericType->isArrayType())
    {
        if (!actualType->isArrayType())
        {
            if (reason.has_value())
            {
                reason =
                    "Type " + actualType->toString() + " is not an array type";
            }

            return false;
        }

        return inferTypeArgsAndMatch(
            ((IRArrayType*)genericType)->base,
            ((IRArrayType*)actualType)->base,
            typeArgs,
            allowGenericSubstitution,
            reason
        );
    }
    else
    {
        if (reason.has_value())
        {
            reason = "Types " + genericType->toString() + " and "
                + actualType->toString() + " do not match";
        }

        return false;
    }
}