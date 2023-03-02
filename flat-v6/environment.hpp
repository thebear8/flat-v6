#include <string>
#include <unordered_map>

namespace llvm
{
class Value;
}

class IRType;
class IRGenericType;
class IRConstraint;
class IRStructTemplate;
class IRStructInstantiation;
class IRFunctionTemplate;
class IRFunctionInstantiation;

/// @brief Manages contained data of an environment, e.g. functions, structs,
/// local variables, type parameters etc.
class Environment
{
protected:
    std::string m_name;
    Environment* m_parent;

    std::unordered_map<std::string, IRType*> m_builtinTypes;

    std::unordered_map<std::string, IRConstraint*> m_constraints;

    std::unordered_map<std::string, IRStructTemplate*> m_structs;
    std::unordered_multimap<IRStructTemplate*, IRStructInstantiation*>
        m_structInstantiations;

    std::unordered_multimap<std::string, IRFunctionTemplate*> m_functions;
    std::unordered_multimap<IRFunctionTemplate*, IRFunctionInstantiation*>
        m_functionInstantiations;

    std::unordered_map<std::string, IRGenericType*> m_typeParams;
    std::unordered_map<IRGenericType*, IRType*> m_typeParamValues;

    std::unordered_map<std::string, IRType*> m_variableTypes;
    std::unordered_map<std::string, llvm::Value*> m_llvmVariableValues;

public:
    Environment(std::string name, Environment* parent)
        : m_name(name), m_parent(parent)
    {
    }

public:
    /// @brief Get the name of this environment
    std::string getName() { return m_name; }

    /// @brief Get the parent of this environment
    Environment* getParent() { return m_parent; }

    /// @brief Add a builtin type to this environment
    /// @param name The name of the builtin type
    /// @param builtinType The type of the builtin type
    /// @return The added builtin type or nullptr if a builtin type with the
    /// same name already exists
    IRType* addBuiltinType(std::string const& name, IRType* builtinType);

    /// @brief Search for a builtin type by name in this environment only
    /// @param name The name of the builtin type
    /// @return The found builtin type or nullptr if the builtin type was not
    /// found
    IRType* getBuiltinType(std::string const& name);

    /// @brief Search for a builtin type by name in the environment chain
    /// @param name The name of the builtin type
    /// @return The found builtin type or nullptr if the builtin type was not
    /// found
    IRType* findBuiltinType(std::string const& name);

    /// @brief Add a generic type parameter to this environment
    /// @param typeParam The generic type parameter to add
    /// @return The added generic type parameter or nullptr if a generic type
    /// parameter with the same name already exists
    IRGenericType* addTypeParam(IRGenericType* typeParam);

    /// @brief Search for a generic type parameter by name in this environment
    /// only
    /// @param name Name of the generic type parameter
    /// @return The found generic type parameter or nullptr if the generic type
    /// parameter was not found
    IRGenericType* getTypeParam(std::string const& name);

    /// @brief Search for a generic type parameter by name in the environment
    /// chain
    /// @param name Name of the generic type parameter
    /// @return The found generic type parameter or nullptr if the type was not
    /// found
    IRGenericType* findTypeParam(std::string const& name);

    /// @brief Add a value for a generic type parameter to this environment
    /// @param typeParam The type parameter to add a value for
    /// @param value The value of the type parameter
    /// @return The added type parameter value or nullptr if a value for the
    /// type parameter is already present
    IRType* addTypeParamValue(IRGenericType* typeParam, IRType* value);

    /// @brief Search for the value of a type parameter in this environment only
    /// @param typeParam Type parameter to search for a value of
    /// @return The found value of the type parameter or nullptr if no value was
    /// found
    IRType* getTypeParamValue(IRGenericType* typeParam);

    /// @brief Search for the value of a type parameter in the environment chain
    /// @param typeParam Type parameter to search for a value of
    /// @return The found value of the type parameter or nullptr if no value was
    /// found
    IRType* findTypeParamValue(IRGenericType* typeParam);

    /// @brief Add a constraint declaration to this environment
    /// @param constraint Constraint declaration to add
    /// @return The added constraint declaration or nullptr if a constraint
    /// declaration with the same name already exists
    IRConstraint* addConstraint(IRConstraint* constraint);

    /// @brief Search for a constraint declaration by name in this environment
    /// only
    /// @param name Name of the constraint declaration
    /// @return The found constraint declaration or nullptr if the constraint
    /// declaration was not found
    IRConstraint* getConstraint(std::string const& name);

    /// @brief Search for a constraint declaration by name in the environment
    /// chain
    /// @param name Name of the constraint declaration
    /// @return The found constraint declaration or nullptr if the constraint
    /// declaration was not found
    IRConstraint* findConstraint(std::string const& name);

    /// @brief Add a struct type to this environment
    /// @param structType Struct type to add
    /// @return The added struct type or nullptr if a struct type with the same
    /// name already exists
    IRStructTemplate* addStruct(IRStructTemplate* structType);

    /// @brief Search for a struct type by name in this environment only
    /// @param name Name of the struct type
    /// @return The found struct type or nullptr if the struct type was not
    /// found
    IRStructTemplate* getStruct(std::string const& name);

    /// @brief Search for a struct type in the environment chain
    /// @param name Name of the struct type
    /// @return The found struct type or nullptr if the struct type was not
    /// found
    IRStructTemplate* findStruct(std::string const& name);

    /// @brief Add an instantiation of a struct to this environment
    /// @param structTemplate The instantiated template
    /// @param structInstantiation The instantiation of the template
    /// @return The added struct instantiaton or nullptr if a struct
    /// instantiation with the same type args already exists
    IRStructInstantiation* addStructInstantiation(
        IRStructTemplate* structTemplate,
        IRStructInstantiation* structInstantiation
    );

    /// @brief Search for a struct instantiation by type args in this
    /// environment only
    /// @param structTemplate The struct template to find an instantiation of
    /// @param typeArgs The type args of the instantiation
    /// @return The found struct instantiation or nullptr if no instantiation
    /// was found
    IRStructInstantiation* getStructInstantiation(
        IRStructTemplate* structTemplate, std::vector<IRType*> const& typeArgs
    );

    /// @brief Search for a struct instantiation by type args in the environment
    /// chain
    /// @param structTemplate The struct template to find an instantiation of
    /// @param typeArgs The type args of the instantiation
    /// @return The found struct instantiation or nullptr if no instantiation
    /// was found
    IRStructInstantiation* findStructInstantiation(
        IRStructTemplate* structTemplate, std::vector<IRType*> const& typeArgs
    );

    /// @brief Add a function with specified name and params to this environment
    /// @param function Function to add
    /// @return The added function or nullptr if a function with the same name
    /// and parameters already exists
    IRFunctionTemplate* addFunction(IRFunctionTemplate* function);

    /// @brief Search for a function by name and params in this environment
    /// @param name Name of the function
    /// @param params Parameters of the function
    /// @return The found function or nullptr if the function was not found
    IRFunctionTemplate* getFunction(
        std::string const& name, std::vector<IRType*> const& params
    );

    /// @brief Search for a function by name and params in the environment chain
    /// @param name Name of the function
    /// @param params Parameters of the function
    /// @return The found function or nullptr if the function was not found
    IRFunctionTemplate* findFunction(
        std::string const& name, std::vector<IRType*> const& params
    );

    /// @brief Add a function instantiation with specified type args to this
    /// environment
    /// @param functionTemplate
    /// @param functionInstantiation
    /// @return
    IRFunctionInstantiation* addFunctionInstantiation(
        IRFunctionTemplate* functionTemplate,
        IRFunctionInstantiation* functionInstantiation
    );

    /// @brief Search for a function instantiation by type args in this
    /// environment
    /// @param functionTemplate The function to get an instantiation of
    /// @param typeArgs The type args of the instantiation
    /// @return The found function instantiation or nullptr if the function
    /// instantiation was not found
    IRFunctionInstantiation* getFunctionInstantiation(
        IRFunctionTemplate* functionTemplate,
        std::vector<IRType*> const& typeArgs
    );

    /// @brief Search for a function instantiation by type args in the
    /// environment chain
    /// @param functionTemplate The function to get an instantiation of
    /// @param typeArgs The type args of the instantiation
    /// @return The found function instantiation or nullptr if the function
    /// instantiation was not found
    IRFunctionInstantiation* findFunctionInstantiation(
        IRFunctionTemplate* functionTemplate,
        std::vector<IRType*> const& typeArgs
    );

    /// @brief Search for a call target by name and argument types
    /// @param name Name of the function
    /// @param params Parameters
    /// @param typeArgs Reference to std::map to which to add the inferred type
    /// parameter values
    /// @return The found function or nullptr if the function was not found
    IRFunctionTemplate* findCallTargetAndInferTypeArgs(
        std::string const& name,
        std::vector<IRType*> const& args,
        std::unordered_map<IRGenericType*, IRType*>& typeArgs
    );

    /// @brief Add a type for a variable of given name to the current
    /// environment
    /// @param name Name of the variable
    /// @param variableType Type of the variable
    /// @return The added type or nullptr if a type for the variable already
    /// exists
    IRType* addVariableType(std::string const& name, IRType* variableType);

    /// @brief Search for a variable's type by name in this environment
    /// @param name Name of the variable
    /// @return The found variable's type or nullptr if the variable was not
    /// found
    IRType* getVariableType(std::string const& name);

    /// @brief Search for a variable's type by name in the environment chain
    /// @param name Name of the variable
    /// @return The found variable's type or nullptr if the variable was not
    /// found
    IRType* findVariableType(std::string const& name);

    /// @brief Set a value for a variable of given name to this environment
    /// @param name The name of the variable
    /// @param variableValue The value of the variable
    /// @return The set value of the variable
    llvm::Value* setVariableValue(std::string const& name, llvm::Value* value);

    /// @brief Search for the value of a variable by name in this environment
    /// @param name The name of the variable
    /// @return The found value of the variable or nullptr if the value was not
    /// found
    llvm::Value* getVariableValue(std::string const& name);

    /// @brief Search for the value of a variable by name in the environment
    /// chain
    /// @param name The name of the variable
    /// @return The found value of the variable or nullptr if the value was not
    /// found
    llvm::Value* findVariableValue(std::string const& name);

public:
    /// @brief Determine if @p actualType is either equal to @p genericType or
    /// an instantiation of @p genericType. If @p actualType is an instantiation
    /// of @p genericType also determine values for type parameters of
    /// @p genericType
    /// @param actualType
    /// @param genericType
    /// @param typeArgs
    /// @return true if @p actualType is compatible with @p genericType,
    /// otherwise false
    bool inferTypeArgsAndMatch(
        IRType* actualType,
        IRType* genericType,
        std::unordered_map<IRGenericType*, IRType*>& typeArgs
    );

    /// @brief Validate that @p actualType is either equal to @p genericType or
    /// an instantiation of @p genericType. If @p actualType is an instantiation
    /// of @p genericType also determine values for type parameters of. If the
    /// validation fails, return a description of why it failed
    /// @p genericType
    /// @param actualType
    /// @param genericType
    /// @param typeArgs
    /// @return std::nullopt if @p actualType is compatible with @p genericType,
    /// otherwise a string describing why @p actualType is not compatible with
    /// @p genericType
    std::optional<std::string> inferTypeArgsAndValidate(
        IRType* genericType,
        IRType* actualType,
        std::unordered_map<IRGenericType*, IRType*>& typeArgs
    );
};