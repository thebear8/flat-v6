#pragma once
#include <set>
#include <vector>

#include "../../ir/ir.hpp"
#include "../../util/optional_ref.hpp"

class GraphContext;
class Instantiator;
class Formatter;
class Environment;
class ConstraintInstantiationUpdatePass;

class CallTargetResolver
{
private:
    Instantiator& m_instantiator;
    ConstraintInstantiationUpdatePass& m_constraintUpdatePass;

public:
    CallTargetResolver(
        Instantiator& instantiator,
        ConstraintInstantiationUpdatePass& constraintUpdatePass
    )
        : m_instantiator(instantiator),
          m_constraintUpdatePass(constraintUpdatePass)
    {
    }

    std::vector<IRFunction*> getMatchingFunctions(
        Environment* env,
        std::string const& name,
        std::vector<IRType*> const& typeArgs,
        std::vector<IRType*> const& args,
        IRType* result,
        optional_ref<std::set<IRFunction*>> argRejected = std::nullopt,
        optional_ref<std::set<IRFunction*>> requirementRejected = std::nullopt
    );

    std::vector<IRFunction*> findMatchingFunctions(
        Environment* env,
        std::string const& name,
        std::vector<IRType*> const& typeArgs,
        std::vector<IRType*> const& args,
        IRType* result,
        optional_ref<std::set<IRFunction*>> argRejected = std::nullopt,
        optional_ref<std::set<IRFunction*>> requirementRejected = std::nullopt
    );

private:
    /// @brief Determine if a constraint is satisfied within the given
    /// environment
    /// @param env The environment to perform the check in
    /// @param constraint The constraint to check
    /// @param reason A string to receive a description of why the constraint is
    /// not satisfied. Will be ignored if set to nullptr
    /// @return true if the constraint is satisfied, false otherwise
    bool isConstraintSatisfied(
        Environment* env, IRConstraintInstantiation* constraint
    );

    std::pair<std::optional<std::vector<IRType*>>, IRFunction*> matchFunction(
        IRFunction* function,
        std::vector<IRType*> const& typeArgs,
        std::vector<IRType*> const& args,
        IRType* result
    );

    bool checkRequirements(
        Environment* env,
        IRFunction* function,
        std::vector<IRType*> const& typeArgs
    );
};