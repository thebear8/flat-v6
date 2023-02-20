#include <string>
#include <unordered_map>

#include "ir/ir.hpp"

/// @brief Manages contained data of an environment, e.g. functions, structs,
/// local variables, generic parameters etc.
class Environment
{
protected:
    std::string m_name;
    Environment* m_parent;

    std::unordered_map<std::string, IRConstraint*> m_constraints;
    std::unordered_map<std::string, IRStructType*> m_structs;
    std::unordered_multimap<std::string, IRFunction*> m_functions;

    std::unordered_map<std::string, IRGenericType*> m_generics;
    std::unordered_map<std::string, IRType*> m_variableTypes;

public:
    Environment(std::string name, Environment* parent)
        : m_name(name), m_parent(parent)
    {
    }

public:
    /// @brief Get the name of this environment
    virtual std::string getName() { return m_name; }

    /// @brief Get the parent of this environment
    virtual Environment* getParent() { return m_parent; }

    /// @brief Search for a type by name in this environment only
    /// @param name Name of the type
    /// @return The found type on success or nullptr if the type was not found
    virtual IRType* getType(std::string const& name);

    /// @brief Search for a type by name in this environment only
    /// @param name Name of the type
    /// @return The found type on success or nullptr if the type was not found
    virtual IRType* findType(std::string const& name);

    /// @brief Add a generic type parameter to this environment
    /// @param genericType The generic type parameter to add
    /// @return The added generic type parameter or nullptr if a generic type
    /// parameter with the same name already exists
    virtual IRGenericType* addGeneric(IRGenericType* genericType);

    /// @brief Search for a generic type parameter by name in this environment
    /// only
    /// @param name Name of the generic type parameter
    /// @return The found generic type parameter or nullptr if the generic type
    /// parameter was not found
    virtual IRGenericType* getGeneric(std::string const& name);

    /// @brief Search for a generic type parameter by name in the environment
    /// chain
    /// @param name Name of the generic type parameter
    /// @return The found generic type parameter or nullptr if the type was not
    /// found
    virtual IRGenericType* findGeneric(std::string const& name);

    /// @brief Add a constraint declaration to this environment
    /// @param constraint Constraint declaration to add
    /// @return The added constraint declaration or nullptr if a constraint
    /// declaration with the same name already exists
    virtual IRConstraint* addConstraint(IRConstraint* constraint);

    /// @brief Search for a constraint declaration by name in this environment
    /// only
    /// @param name Name of the constraint declaration
    /// @return The found constraint declaration or nullptr if the constraint
    /// declaration was not found
    virtual IRConstraint* getConstraint(std::string const& name);

    /// @brief Search for a constraint declaration by name in the environment
    /// chain
    /// @param name Name of the constraint declaration
    /// @return The found constraint declaration or nullptr if the constraint
    /// declaration was not found
    virtual IRConstraint* findConstraint(std::string const& name);

    /// @brief Add a struct type to this environment
    /// @param structType Struct type to add
    /// @return The added struct type or nullptr if a struct type with the same
    /// name already exists
    virtual IRStructType* addStruct(IRStructType* structType);

    /// @brief Search for a struct type by name in this environment only
    /// @param name Name of the struct type
    /// @return The found struct type or nullptr if the struct type was not
    /// found
    virtual IRStructType* getStruct(std::string const& name);

    /// @brief Search for a struct type in the environment chain
    /// @param name Name of the struct type
    /// @return The found struct type or nullptr if the struct type was not
    /// found
    virtual IRStructType* findStruct(std::string const& name);

    /// @brief Add a function with specified name and params to this environment
    /// @param function Function to add
    /// @return The added function or nullptr if a function with the same name
    /// and parameters already exists
    virtual IRFunction* addFunction(IRFunction* function);

    /// @brief Search for a function by name and params in this environment
    /// @param name Name of the function
    /// @param params Parameters of the function
    /// @return The found function or nullptr if the function was not found
    virtual IRFunction* getFunction(
        std::string const& name, std::vector<IRType*> const& params
    );

    /// @brief Search for a function by name and params in the environment chain
    /// @param name Name of the function
    /// @param params Parameters of the function
    /// @return The found function or nullptr if the function was not found
    virtual IRFunction* findFunction(
        std::string const& name, std::vector<IRType*> const& params
    );

    /// @brief Add a type for a variable of given name to the current
    /// environment
    /// @param name Name of the variable
    /// @param variableType Type of the variable
    /// @return The added type or nullptr if a type for the variable already
    /// exists
    virtual IRType* addVariableType(
        std::string const& name, IRType* variableType
    );

    /// @brief Search for a variable's type by name in this environment
    /// @param name Name of the variable
    /// @return The found variable's type or nullptr if the variable was not
    /// found
    virtual IRType* getVariableType(std::string const& name);

    /// @brief Search for a variable's type by name in the environment chain
    /// @param name Name of the variable
    /// @return The found variable's type or nullptr if the variable was not
    /// found
    virtual IRType* findVariableType(std::string const& name);
};