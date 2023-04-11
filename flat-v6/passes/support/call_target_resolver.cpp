#include "call_target_resolver.hpp"

#include "../../environment.hpp"
#include "../../support/formatter.hpp"
#include "../../util/assert.hpp"
#include "../../util/graph_context.hpp"
#include "../../util/to_vector.hpp"
#include "../../util/zip_view.hpp"

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
                && !env->inferTypeArgsAndMatch(param, arg, typeArgMap, true))
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

    auto [it, end] = env->getFunctionTemplateMap().equal_range(name);
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

    auto candidatesVec = candidates | range_utils::to_vector;

    auto requirementCompatibleCandidates =
        candidates | std::views::filter([&](auto const& f) {
            return std::ranges::all_of(f.second->requirements, [&](auto r) {
                return isConstraintSatisfied(env, r);
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