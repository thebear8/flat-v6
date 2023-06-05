#include "call_target_resolver.hpp"

#include <set>

#include "../../environment.hpp"
#include "../../support/formatter.hpp"
#include "../../util/assert.hpp"
#include "../../util/graph_context.hpp"
#include "../../util/to_vector.hpp"
#include "../../util/zip_view.hpp"
#include "../update/constraint_instantiation_update_pass.hpp"
#include "instantiator.hpp"

std::pair<std::optional<std::vector<IRType*>>, IRFunction*>
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
            return { std::nullopt, function };
    }

    std::vector<IRType*> typeArgList;
    for (auto typeParam : function->typeParams)
    {
        if (!typeArgMap.contains(typeParam))
            return { std::nullopt, function };

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
    optional_ref<std::set<IRFunction*>> argRejected,
    optional_ref<std::set<IRFunction*>> requirementRejected
)
{
    auto [it, end] = env->getFunctionMap().equal_range(name);
    auto candidates = std::ranges::subrange(it, end) | std::views::values
        | std::views::filter([&](IRFunction* f) {
                          return (typeArgs.size() <= f->typeParams.size())
                              && (args.size() == f->params.size());
                      })
        | std::views::transform([&](IRFunction* f) {
                          return matchFunction(f, typeArgs, args, result);
                      })
        | std::views::filter([&](auto const& f) {
                          (!f.first && argRejected
                           && (argRejected->emplace(f.second), true));
                          return f.first.has_value();
                      })
        | std::views::transform([&](auto const& f) {
                          return std::pair(f.first.value(), f.second);
                      })
        | std::views::filter([&](auto const& f) {
                          auto r = checkRequirements(env, f.second, f.first);
                          (!r && requirementRejected
                           && (requirementRejected->emplace(f.second), true));
                          return r;
                      })
        | range_utils::to_vector;

    std::ranges::sort(candidates, [](auto const& a, auto const& b) {
        return a.first.size() > b.first.size();
    });

    return candidates | std::views::transform([&](auto const& f) {
               auto function = f.second;
               auto functionInstantiation =
                   m_instantiator.getFunctionInstantiation(function, f.first);

               // We have to add the function instantiation to the parent env
               // here, as we
               // only now know that the instantiation is legal.
               // If we do so in the instantiator, we are going to get errors
               // later
               if (functionInstantiation != function)
               {
                   function->parent->getEnv()->addFunctionInstantiation(
                       function, functionInstantiation
                   );
               }

               return functionInstantiation;
           })
        | range_utils::to_vector;
}

std::vector<IRFunction*> CallTargetResolver::findMatchingFunctions(
    Environment* env,
    std::string const& name,
    std::vector<IRType*> const& typeArgs,
    std::vector<IRType*> const& args,
    IRType* result,
    optional_ref<std::set<IRFunction*>> argRejected,
    optional_ref<std::set<IRFunction*>> requirementRejected
)
{
    std::vector<std::pair<std::vector<IRType*>, IRFunction*>> candidates;
    while (env != nullptr)
    {
        auto [it, end] = env->getFunctionMap().equal_range(name);
        std::ranges::for_each(
            std::ranges::subrange(it, end) | std::views::values
                | std::views::filter([&](IRFunction* f) {
                      return (typeArgs.size() <= f->typeParams.size())
                          && (args.size() == f->params.size());
                  })
                | std::views::transform([&](IRFunction* f) {
                      return matchFunction(f, typeArgs, args, result);
                  })
                | std::views::filter([&](auto const& f) {
                      (!f.first && argRejected
                       && (argRejected->emplace(f.second), true));
                      return f.first.has_value();
                  })
                | std::views::transform([&](auto const& f) {
                      return std::pair(f.first.value(), f.second);
                  })
                | std::views::filter([&](auto const& f) {
                      auto r = checkRequirements(env, f.second, f.first);
                      (!r && requirementRejected
                       && (requirementRejected->emplace(f.second), true));
                      return r;
                  }),
            [&](auto const& f) {
            candidates.push_back(f);
            });

        env = env->getParent();
    }

    std::ranges::sort(candidates, [](auto const& a, auto const& b) {
        return a.first.size() > b.first.size();
    });

    return candidates | std::views::transform([&](auto const& f) {
               auto function = f.second;
               auto functionInstantiation =
                   m_instantiator.getFunctionInstantiation(function, f.first);

               // We have to add the function instantiation to the parent env
               // here, as we
               // only now know that the instantiation is legal.
               // If we do so in the instantiator, we are going to get errors
               // later
               if (functionInstantiation != function)
               {
                   function->parent->getEnv()->addFunctionInstantiation(
                       function, functionInstantiation
                   );
               }

               return functionInstantiation;
           })
        | range_utils::to_vector;
}

bool CallTargetResolver::isConstraintSatisfied(
    Environment* env, IRConstraintInstantiation* constraint
)
{
    m_constraintUpdatePass.update(constraint);

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