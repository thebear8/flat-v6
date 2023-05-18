#include "call_target_resolver.hpp"

#include <set>

#include "../../environment.hpp"
#include "../../support/formatter.hpp"
#include "../../util/assert.hpp"
#include "../../util/graph_context.hpp"
#include "../../util/to_vector.hpp"
#include "../../util/zip_view.hpp"
#include "instantiator.hpp"

std::optional<std::pair<std::vector<IRType*>, IRFunction*>>
CallTargetResolver::matchFunction(
    IRFunction* function,
    std::vector<IRType*> const& typeArgs,
    std::vector<IRType*> const& args,
    IRType* result
)
{
    auto zippedTypeArgs = zip(function->typeParams, typeArgs);
    auto typeArgMap =
        std::unordered_map(zippedTypeArgs.begin(), zippedTypeArgs.end());

    auto zippedArgs = zip(function->params | std::views::values, args);
    for (auto [param, arg] : zippedArgs)
    {
        if (param != arg
            && !Environment::inferTypeArgsAndMatch(
                param, arg, typeArgMap, true
            ))
            return std::nullopt;
    }

    std::vector<IRType*> typeArgList;
    for (auto typeParam : function->typeParams)
    {
        if (!typeArgMap.contains(typeParam))
            return std::nullopt;

        typeArgList.push_back(typeArgMap.at(typeParam));
    }

    return std::pair(typeArgList, function);
}

bool CallTargetResolver::checkRequirements(
    Environment* env, IRFunction* function, std::vector<IRType*> const& typeArgs
)
{
    auto functionInstantiation =
        m_instantiator.getFunctionInstantiation(function, typeArgs);

    for (auto r : functionInstantiation->requirements)
    {
        if (!isConstraintSatisfied(env, r))
            return false;
    }

    return true;
}

std::vector<IRFunction*> CallTargetResolver::getMatchingFunctions(
    Environment* env,
    std::string const& name,
    std::vector<IRType*> const& typeArgs,
    std::vector<IRType*> const& args,
    IRType* result,
    optional_ref<std::vector<IRType*>> inferredTypeArgs,
    optional_ref<std::set<IRFunction*>> argRejected,
    optional_ref<std::set<IRFunction*>> requirementRejected
)
{
    auto [it, end] = env->getFunctionMap().equal_range(name);
    auto candidates =
        std::ranges::subrange(it, end) | std::views::values
        | std::views::filter([&](IRFunction* f) {
              return (typeArgs.size() <= f->typeParams.size())
                  && (args.size() == f->params.size());
          })
        | std::views::transform([&](IRFunction* f) {
              return matchFunction(f, typeArgs, args, result);
          })
        | std::views::filter([&](auto const& f) {
              (!f && argRejected && (argRejected->emplace(f), true));
              return f.has_value();
          })
        | std::views::transform([&](auto const& f) {
              return f.value();
          })
        | std::views::filter([&](auto const& f) {
              auto r = checkRequirements(env, f.second, f.first);
              (!r && requirementRejected
               && (requirementRejected->emplace(f), true));
              return r;
          })
        | range_utils::to_vector;

    std::ranges::sort(candidates, [](auto const& a, auto const& b) {
        return a.first.size() > b.first.size();
    });

    return candidates | std::views::values | range_utils::to_vector;
}

bool CallTargetResolver::isConstraintSatisfied(
    Environment* env, IRConstraintInstantiation* constraint
)
{
    for (auto requirement : constraint->requirements)
    {
        if (!isConstraintSatisfied(env, requirement))
            return false;
    }

    for (auto condition : constraint->conditions)
    {
        auto candidates = getMatchingFunctions(
            env,
            condition->name,
            {},
            condition->params | std::views::values | range_utils::to_vector,
            condition->result
        );

        if (candidates.size() != 1)
            return false;
    }

    return true;
}