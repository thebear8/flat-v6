#include "../../ir/ir.hpp"
#include "../../util/optional_ref.hpp"

class GraphContext;
class Instantiator;
class Formatter;
class Environment;

class CallTargetResolver : IRRefVisitor<IRNode*>
{
private:
    Instantiator& m_instantiator;
    Formatter& m_formatter;

public:
    CallTargetResolver(Instantiator& instantiator, Formatter& formatter)
        : m_instantiator(instantiator), m_formatter(formatter)
    {
    }

    /// @brief Search for a constraint condition by name, params and optionally
    /// result type in the given environment without considering the parent
    /// environment
    /// @param name Name of the constraint condition
    /// @param args Arguments that the params of the constraint condition have
    /// to match
    /// @param result Result type of the constraint condition. Will be ignored
    /// if set to nullptr
    /// @param reason A reference to a string to receive a description of why no
    /// matching constraint condition was found. Will be ignored if set to
    /// nullopt
    /// @return The found constraint condition or nullptr if no constraint
    /// condition was found
    IRFunctionHead* getMatchingConstraintCondition(
        Environment* env,
        std::string const& name,
        std::vector<IRType*> args,
        IRType* result = nullptr,
        optional_ref<std::string> reason = std::nullopt
    );

    /// @brief Search for a constraint condition by name, params and optionally
    /// result type in the given environment chain
    /// @param name Name of the constraint condition
    /// @param args Arguments that the params of the constraint condition have
    /// to match
    /// @param result Result type of the constraint condition. Will be ignored
    /// if set to nullptr
    /// @param reason A reference to a string to receive a description of why no
    /// matching constraint condition was found. Will be ignored if set to
    /// nullopt
    /// @return The found constraint condition or nullptr if no constraint
    /// condition was found
    IRFunctionHead* findMatchingConstraintCondition(
        Environment* env,
        std::string const& name,
        std::vector<IRType*> args,
        IRType* result = nullptr,
        optional_ref<std::string> reason = std::nullopt
    );

    /// @brief Search for a matching function template by name, type args, args
    /// and optionally return type in the given environment without considering
    /// the parent environment
    /// @param name Name of the function template
    /// @param typeArgs Type arguments to match the type parameters of the
    /// function template to
    /// @param args Arguments to match the parameters of the function template
    /// to
    /// @param result Result type of the function template. Will be ignored if
    /// set to nullptr
    /// @param reason A reference to a string to receive a description of why no
    /// matching function template was found
    /// @param inferredTypeArgs A reference to a vector<IRType*> to which the
    /// inferred type parameters of the function template shall be appended.
    /// Will be ignored if set to std::nullopt
    /// @return The found function template or nullptr if no function template
    /// was found
    IRFunctionTemplate* getMatchingFunctionTemplate(
        Environment* env,
        std::string const& name,
        std::vector<IRType*> const& typeArgs,
        std::vector<IRType*> const& args,
        IRType* result = nullptr,
        optional_ref<std::vector<IRType*>> inferredTypeArgs = std::nullopt,
        optional_ref<std::string> reason = std::nullopt
    );

    /// @brief Search for a matching function template by name, type args, args
    /// and optionally return type in the given environment chain
    /// @param env The environment to search in
    /// @param name Name of the function template
    /// @param typeArgs Type arguments to match the type parameters of the
    /// function template to
    /// @param args Arguments to match the parameters of the function template
    /// to
    /// @param result Result type of the function template. Will be ignored if
    /// set to nullptr
    /// @param reason A reference to a string to receive a description of why no
    /// matching function template was found
    /// @param inferredTypeArgs A reference to a vector<IRType*> to which the
    /// inferred type parameters of the function template shall be appended.
    /// Will be ignored if set to std::nullopt
    /// @return The found function template or nullptr if no function template
    /// was found
    IRFunctionTemplate* findMatchingFunctionTemplate(
        Environment* env,
        std::string const& name,
        std::vector<IRType*> const& typeArgs,
        std::vector<IRType*> const& args,
        IRType* result = nullptr,
        optional_ref<std::vector<IRType*>> inferredTypeArgs = std::nullopt,
        optional_ref<std::string> reason = std::nullopt
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
        Environment* env,
        IRConstraintInstantiation* constraint,
        optional_ref<std::string> reason = std::nullopt
    );

    std::optional<std::pair<std::vector<IRType*>, IRFunctionTemplate*>>
    matchFunctionTemplate(
        IRFunctionTemplate* functionTemplate,
        std::vector<IRType*> const& typeArgs,
        std::vector<IRType*> const& args,
        IRType* result
    );

    bool checkRequirements(
        Environment* env,
        IRFunctionTemplate* functionTemplate,
        std::vector<IRType*> const& typeArgs
    );
};