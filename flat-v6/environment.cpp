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

IRFunctionHead* Environment::addConstraintCondition(IRFunctionHead* condition)
{
    auto params = condition->params | std::views::transform([](auto const& p) {
                      return p.second;
                  })
        | range_utils::to_vector;

    if (getConstraintCondition(condition->name, params))
        return nullptr;

    m_constraintConditions.insert(std::pair(condition->name, condition));
    return condition;
}

IRFunctionHead* Environment::getMatchingConstraintCondition(
    std::string const& name,
    std::vector<IRType*> args,
    IRType* result,
    optional_ref<std::string> reason
)
{
    auto [it, end] = m_constraintConditions.equal_range(name);
    auto conditions = std::ranges::subrange(it, end) | std::views::values;
    auto matchingConditions =
        conditions | std::views::filter([&](auto c) {
            auto params = c->params | std::views::values;

            return (
                params.size() == args.size() && std::ranges::equal(params, args)
                && (!result || (c->result == result))
            );
        })
        | range_utils::to_vector;

    if (matchingConditions.size() == 1)
    {
        return matchingConditions.front();
    }
    else if (matchingConditions.size() == 0)
    {
        if (reason.has_value())
        {
            reason = "No matching constraint condition for "
                + m_formatter.formatCallDescriptor(name, {}, args, result);
        }

        return nullptr;
    }
    else
    {
        if (reason.has_value())
        {
            reason = "Multiple matching constraint conditions for "
                + m_formatter.formatCallDescriptor(name, {}, args, result);

            for (auto c : matchingConditions)
            {
                reason.get() += "\n  Candidate: "
                    + m_formatter.formatFunctionHeadDescriptor(c);
            }
        }

        return nullptr;
    }
}

IRFunctionHead* Environment::findMatchingConstraintCondition(
    std::string const& name,
    std::vector<IRType*> args,
    IRType* result,
    optional_ref<std::string> reason
)
{
    if (auto c = getMatchingConstraintCondition(name, args, result, reason))
    {
        return c;
    }
    else if (m_parent)
    {
        return m_parent->findMatchingConstraintCondition(
            name, args, result, reason
        );
    }
    else
    {
        return nullptr;
    }
}

IRFunctionHead* Environment::getConstraintCondition(
    std::string const& name, std::vector<IRType*> const& params
)
{
    for (auto [i, end] = m_constraintConditions.equal_range(name); i != end;
         ++i)
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

IRFunctionHead* Environment::findConstraintCondition(
    std::string const& name, std::vector<IRType*> const& params
)
{
    if (auto c = getConstraintCondition(name, params))
        return c;
    else if (m_parent)
        return m_parent->findConstraintCondition(name, params);

    return nullptr;
}

IRFunctionTemplate* Environment::addFunctionTemplate(
    IRFunctionTemplate* function
)
{
    auto params = function->params | std::views::transform([](auto const& p) {
                      return p.second;
                  })
        | range_utils::to_vector;

    if (getFunctionTemplate(function->name, params))
        return nullptr;

    m_functionTemplates.insert(std::pair(function->name, function));
    return function;
}

IRFunctionTemplate* Environment::getMatchingFunctionTemplate(
    std::string const& name,
    std::vector<IRType*> const& typeArgs,
    std::vector<IRType*> const& args,
    IRType* result,
    optional_ref<std::vector<IRType*>> inferredTypeArgs,
    optional_ref<std::string> reason
)
{
    auto matchFunctionTemplate =
        [&](IRFunctionTemplate* f,
            optional_ref<std::vector<IRType*>> inferredTypeArgs =
                std::nullopt) {
        zip_view zippedTypeArgs(f->typeParams, typeArgs);
        std::unordered_map typeArgMap(
            zippedTypeArgs.begin(), zippedTypeArgs.end()
        );

        zip_view zippedArgs(f->params | std::views::values, args);
        for (auto [param, arg] : zippedArgs)
        {
            if (param != arg
                && !inferTypeArgsAndMatch(param, arg, typeArgMap, true))
            {
                return std::pair(false, std::pair((size_t)0, f));
            }
        }

        for (auto typeParam : f->typeParams)
        {
            if (!typeArgMap.contains(typeParam))
                return std::pair(false, std::pair((size_t)0, f));

            if (inferredTypeArgs.has_value())
                inferredTypeArgs.get().push_back(typeArgMap.at(typeParam));
        }

        return std::pair(true, std::pair(typeArgMap.size(), f));
    };

    auto [it, end] = m_functionTemplates.equal_range(name);
    auto candidates = std::ranges::subrange(it, end) | std::views::values
        | std::views::filter([&](auto f) {
                          return (typeArgs.size() <= f->typeParams.size())
                              && (args.size() == f->params.size());
                      })
        | std::views::transform([&](auto f) {
                          return matchFunctionTemplate(f);
                      })
        | std::views::filter([&](auto const& f) {
                          return f.first;
                      })
        | std::views::transform([&](auto const& f) {
                          return f.second;
                      });

    auto requirementCompatibleCandidates =
        candidates | std::views::filter([&](auto const& f) {
            return std::ranges::all_of(f.second->requirements, [&](auto r) {
                return isConstraintSatisfied(r);
            });
        })
        | range_utils::to_vector;

    if (requirementCompatibleCandidates.size() == 0)
    {
        if (reason.has_value())
        {
            reason = "No function template matching "
                + m_formatter.formatCallDescriptor(
                    name, typeArgs, args, result
                );
        }

        return nullptr;
    }

    std::ranges::sort(
        requirementCompatibleCandidates,
        [](auto const& a, auto const& b) {
        return a.second > b.second;
        });

    std::unordered_multimap functionTemplateMap(
        requirementCompatibleCandidates.begin(),
        requirementCompatibleCandidates.end()
    );

    if (functionTemplateMap.count(requirementCompatibleCandidates.front().first)
        > 1)
    {
        if (reason.has_value())
        {
            reason = "Multiple function templates matching "
                + m_formatter.formatCallDescriptor(
                    name, typeArgs, args, result
                );
        }

        return nullptr;
    }

    auto target = requirementCompatibleCandidates.front().second;
    if (inferredTypeArgs.has_value())
        matchFunctionTemplate(target, inferredTypeArgs.get());

    return target;
}

IRFunctionTemplate* Environment::findMatchingFunctionTemplate(
    std::string const& name,
    std::vector<IRType*> const& typeArgs,
    std::vector<IRType*> const& args,
    IRType* result,
    optional_ref<std::vector<IRType*>> inferredTypeArgs,
    optional_ref<std::string> reason
)
{
    if (auto f = getMatchingFunctionTemplate(
            name, typeArgs, args, result, inferredTypeArgs, reason
        ))
    {
        return f;
    }
    else if (m_parent)
    {
        return m_parent->getMatchingFunctionTemplate(
            name, typeArgs, args, result, inferredTypeArgs, reason
        );
    }
    else
    {
        return nullptr;
    }
}

IRFunctionTemplate* Environment::getFunctionTemplate(
    std::string const& functionName, std::vector<IRType*> const& params
)
{
    for (auto [i, end] = m_functionTemplates.equal_range(functionName);
         i != end;
         ++i)
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

IRFunctionTemplate* Environment::findFunctionTemplate(
    std::string const& functionName, std::vector<IRType*> const& params
)
{
    if (auto f = getFunctionTemplate(functionName, params))
        return f;
    else if (m_parent)
        return m_parent->findFunctionTemplate(functionName, params);

    return nullptr;
}

IRFunctionInstantiation* Environment::addFunctionInstantiation(
    IRFunctionTemplate* functionTemplate,
    IRFunctionInstantiation* functionInstantiation
)
{
    auto const& typeArgs = functionInstantiation->typeArgs;
    if (getFunctionInstantiation(functionTemplate, typeArgs))
        return nullptr;

    m_functionInstantiations.emplace(functionTemplate, functionInstantiation);
    return functionInstantiation;
}

IRFunctionInstantiation* Environment::getFunctionInstantiation(
    IRFunctionTemplate* functionTemplate, std::vector<IRType*> const& typeArgs
)
{
    for (auto [it, end] =
             m_functionInstantiations.equal_range(functionTemplate);
         it != end;
         ++it)
    {
        if (std::ranges::equal(it->second->typeArgs, typeArgs))
            return it->second;
    }

    return nullptr;
}

IRFunctionInstantiation* Environment::findFunctionInstantiation(
    IRFunctionTemplate* functionTemplate, std::vector<IRType*> const& typeArgs
)
{
    if (auto i = getFunctionInstantiation(functionTemplate, typeArgs))
        return i;
    else if (m_parent)
        return m_parent->findFunctionInstantiation(functionTemplate, typeArgs);

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

bool Environment::isConstraintSatisfied(
    IRConstraintInstantiation* constraint, optional_ref<std::string> reason
)
{
    for (auto requirement : constraint->requirements)
    {
        if (!isConstraintSatisfied(requirement, reason))
        {
            if (reason.has_value())
            {
                reason = "Requirement "
                    + m_formatter.formatConstraintInstantiationDescriptor(
                        requirement
                    )
                    + " is not satisfied: " + reason.get();
            }

            return false;
        }
    }

    for (auto condition : constraint->conditions)
    {
        auto args =
            condition->params | std::views::values | range_utils::to_vector;
        auto target = findMatchingFunctionTemplate(
            condition->name, {}, args, condition->result, std::nullopt, reason
        );

        if (!target)
        {
            if (reason.has_value())
            {
                reason = "No matching target for condition "
                    + m_formatter.formatFunctionHeadDescriptor(condition) + ": "
                    + reason.get();
            }

            return false;
        }
    }

    return true;
}