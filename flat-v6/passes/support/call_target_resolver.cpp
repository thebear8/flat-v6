#include "call_target_resolver.hpp"

#include <set>

#include "../../environment.hpp"
#include "../../support/formatter.hpp"
#include "../../util/assert.hpp"
#include "../../util/graph_context.hpp"
#include "../../util/to_vector.hpp"
#include "../../util/zip_view.hpp"
#include "instantiator.hpp"

IRFunctionHead* CallTargetResolver::getMatchingConstraintCondition(
    Environment* env,
    std::string const& name,
    std::vector<IRType*> args,
    IRType* result,
    optional_ref<std::string> reason
)
{
    auto [it, end] = env->getConstraintConditionMap().equal_range(name);
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

IRFunctionHead* CallTargetResolver::findMatchingConstraintCondition(
    Environment* env,
    std::string const& name,
    std::vector<IRType*> args,
    IRType* result,
    optional_ref<std::string> reason
)
{
    if (auto c =
            getMatchingConstraintCondition(env, name, args, result, reason))
    {
        return c;
    }
    else if (env->getParent())
    {
        return findMatchingConstraintCondition(
            env->getParent(), name, args, result, reason
        );
    }
    else
    {
        return nullptr;
    }
}

std::optional<std::pair<std::vector<IRType*>, IRFunctionTemplate*>>
CallTargetResolver::matchFunctionTemplate(
    IRFunctionTemplate* functionTemplate,
    std::vector<IRType*> const& typeArgs,
    std::vector<IRType*> const& args,
    IRType* result
)
{
    zip_view zippedTypeArgs(functionTemplate->typeParams, typeArgs);
    std::unordered_map typeArgMap(zippedTypeArgs.begin(), zippedTypeArgs.end());

    zip_view zippedArgs(functionTemplate->params | std::views::values, args);
    for (auto [param, arg] : zippedArgs)
    {
        if (param != arg
            && !Environment::inferTypeArgsAndMatch(
                param, arg, typeArgMap, true
            ))
        {
            return std::nullopt;
        }
    }

    std::vector<IRType*> typeArgList;
    for (auto typeParam : functionTemplate->typeParams)
    {
        if (!typeArgMap.contains(typeParam))
            return std::nullopt;

        typeArgList.push_back(typeArgMap.at(typeParam));
    }

    return std::pair(typeArgList, functionTemplate);
}

bool CallTargetResolver::checkRequirements(
    Environment* env,
    IRFunctionTemplate* functionTemplate,
    std::vector<IRType*> const& typeArgs
)
{
    auto functionInstantiation =
        m_instantiator.getFunctionInstantiation(functionTemplate, typeArgs);

    for (auto r : functionInstantiation->requirements)
    {
        if (!isConstraintSatisfied(env, r))
            return false;
    }

    return true;
}

IRFunctionTemplate* CallTargetResolver::getMatchingFunctionTemplate(
    Environment* env,
    std::string const& name,
    std::vector<IRType*> const& typeArgs,
    std::vector<IRType*> const& args,
    IRType* result,
    optional_ref<std::vector<IRType*>> inferredTypeArgs,
    optional_ref<std::string> reason
)
{
    auto [it, end] = env->getFunctionTemplateMap().equal_range(name);
    auto candidates =
        std::ranges::subrange(it, end) | std::views::values
        | std::views::filter([&](IRFunctionTemplate* f) {
              return (typeArgs.size() <= f->typeParams.size())
                  && (args.size() == f->params.size());
          })
        | std::views::transform([&](IRFunctionTemplate* f) {
              return matchFunctionTemplate(f, typeArgs, args, result);
          })
        | std::views::filter([&](auto const& f) {
              return f.has_value();
          })
        | std::views::transform([&](auto const& f) {
              return f.value();
          });

    auto requirementCompatibleCandidates =
        candidates | std::views::filter([&](auto const& f) {
            return checkRequirements(env, f.second, f.first);
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
        return a.first.size() > b.first.size();
        });

    std::multiset<size_t> substitutionCount;
    for (auto const& [typeArgs, functionTemplate] :
         requirementCompatibleCandidates)
    {
        substitutionCount.emplace(typeArgs.size());
    }

    if (substitutionCount.count(
            requirementCompatibleCandidates.front().first.size()
        )
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

    if (inferredTypeArgs.has_value())
    {
        inferredTypeArgs.get() =
            std::vector(requirementCompatibleCandidates.front().first);
    }

    return requirementCompatibleCandidates.front().second;
}

IRFunctionTemplate* CallTargetResolver::findMatchingFunctionTemplate(
    Environment* env,
    std::string const& name,
    std::vector<IRType*> const& typeArgs,
    std::vector<IRType*> const& args,
    IRType* result,
    optional_ref<std::vector<IRType*>> inferredTypeArgs,
    optional_ref<std::string> reason
)
{
    if (auto f = getMatchingFunctionTemplate(
            env, name, typeArgs, args, result, inferredTypeArgs, reason
        ))
    {
        return f;
    }
    else if (env->getParent())
    {
        return findMatchingFunctionTemplate(
            env->getParent(),
            name,
            typeArgs,
            args,
            result,
            inferredTypeArgs,
            reason
        );
    }
    else
    {
        return nullptr;
    }
}

bool CallTargetResolver::isConstraintSatisfied(
    Environment* env,
    IRConstraintInstantiation* constraint,
    optional_ref<std::string> reason
)
{
    for (auto requirement : constraint->requirements)
    {
        if (!isConstraintSatisfied(env, requirement, reason))
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
            env,
            condition->name,
            {},
            args,
            condition->result,
            std::nullopt,
            reason
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