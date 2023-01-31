#include <string>
#include <unordered_map>

#include "data/ast.hpp"
#include "data/ir.hpp"
#include "data/type.hpp"

/// @brief Manages contained data of an environment, e.g. functions, structs,
/// local variables, generic parameters etc.
class Environment
{
protected:
    std::string m_name;
    Environment* m_parent;

    std::unordered_map<std::string, StructType*> m_structs;
    std::unordered_multimap<std::string, IRFunctionDeclaration*> m_functions;

    std::unordered_map<std::string, GenericType*> m_generics;
    std::unordered_map<std::string, Type*> m_variableTypes;

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
    virtual Type* getType(std::string const& name);

    /// @brief Search for a type by name in this environment only
    /// @param name Name of the type
    /// @return The found type on success or nullptr if the type was not found
    virtual Type* findType(std::string const& name);

    /// @brief Add a generic type parameter to this environment
    /// @param genericType The generic type parameter to add
    /// @return The added generic type parameter or nullptr if a generic type
    /// parameter with the same name already exists
    virtual GenericType* addGeneric(GenericType* genericType);

    /// @brief Search for a generic type parameter by name in this environment
    /// only
    /// @param name Name of the generic type parameter
    /// @return The found generic type parameter or nullptr if the generic type
    /// parameter was not found
    virtual GenericType* getGeneric(std::string const& name);

    /// @brief Search for a generic type parameter by name in the environment
    /// chain
    /// @param name Name of the generic type parameter
    /// @return The found generic type parameter or nullptr if the type was not
    /// found
    virtual GenericType* findGeneric(std::string const& name);

    /// @brief Add a struct type to this environment
    /// @param name Name of the struct type
    /// @return The added struct type or nullptr if a struct type with the same
    /// name already exists
    virtual StructType* addStruct(StructType* structType);

    /// @brief Search for a struct type by name in this environment only
    /// @param name Name of the struct type
    /// @return The found struct type or nullptr if the struct type was not
    /// found
    virtual StructType* getStruct(std::string const& name);

    /// @brief Search for a struct type in the environment chain
    /// @param name Name of the struct type
    /// @return The found struct type or nullptr if the struct type was not
    /// found
    virtual StructType* findStruct(std::string const& name);

    /// @brief Add a function with specified name and params to this environment
    /// @param function Function to add
    /// @return The added function or nullptr if a function with the same name
    /// and parameters already exists
    virtual IRFunctionDeclaration* addFunction(IRFunctionDeclaration* function);

    /// @brief Search for a function by name and params in this environment
    /// @param name Name of the function
    /// @param params Parameters of the function
    /// @return The found function or nullptr if the function was not found
    virtual IRFunctionDeclaration* getFunction(
        std::string const& name, std::vector<Type*> const& params);

    /// @brief Search for a function by name and params in the environment chain
    /// @param name Name of the function
    /// @param params Parameters of the function
    /// @return The found function or nullptr if the function was not found
    virtual IRFunctionDeclaration* findFunction(
        std::string const& name, std::vector<Type*> const& params);

    /// @brief Add a type for a variable of given name to the current
    /// environment
    /// @param name Name of the variable
    /// @param variableType Type of the variable
    /// @return The added type or nullptr if a type for the variable already
    /// exists
    virtual Type* addVariableType(std::string const& name, Type* variableType);

    /// @brief Search for a variable's type by name in this environment
    /// @param name Name of the variable
    /// @return The found variable's type or nullptr if the variable was not
    /// found
    virtual Type* getVariableType(std::string const& name);

    /// @brief Search for a variable's type by name in the environment chain
    /// @param name Name of the variable
    /// @return The found variable's type or nullptr if the variable was not
    /// found
    virtual Type* findVariableType(std::string const& name);
};