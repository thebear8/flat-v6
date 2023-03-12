#include <optional>
#include <string>
#include <unordered_map>

namespace llvm
{
class Value;
}

class IRType;
class IRGenericType;
class IRConstraintTemplate;
class IRConstraintInstantiation;
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

    std::unordered_map<std::string, IRConstraintTemplate*> m_constraints;
    std::unordered_multimap<IRConstraintTemplate*, IRConstraintInstantiation*>
        m_constraintInstantiations;

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

    /// @brief Add a constraint template to this environment
    /// @param constraint Constraint template to add
    /// @return The added constraint template or nullptr if a constraint
    /// template with the same name already exists
    IRConstraintTemplate* addConstraint(IRConstraintTemplate* constraint);

    /// @brief Search for a constraint template by name in this environment
    /// only
    /// @param name Name of the constraint template
    /// @return The found constraint template or nullptr if the constraint
    /// template was not found
    IRConstraintTemplate* getConstraint(std::string const& name);

    /// @brief Search for a constraint template by name in the environment
    /// chain
    /// @param name Name of the constraint template
    /// @return The found constraint template or nullptr if the constraint
    /// template was not found
    IRConstraintTemplate* findConstraint(std::string const& name);

    /// @brief Add a constraint instantiation to this environment
    /// @param constraintTemplate Constraint template for which to add an
    /// instantiation
    /// @param constraintInstantiation Constraint instantiation to add
    /// @return The added instantiation or nullptr if an instantiation with the
    /// same type args already exists
    IRConstraintInstantiation* addConstraintInstantiation(
        IRConstraintTemplate* constraintTemplate,
        IRConstraintInstantiation* constraintInstantiation
    );

    /// @brief Search for a constraint instantiation by type args in this
    /// environment only
    /// @param constraintTemplate Constraint template for which to get an
    /// instantiation
    /// @param typeArgs The type args of the instantiation to get
    /// @return The found instantiation or nullptr if no instantiation was found
    IRConstraintInstantiation* getConstraintInstantiation(
        IRConstraintTemplate* constraintTemplate,
        std::vector<IRType*> const& typeArgs
    );

    /// @brief Search for a constraint instantiation by type args in the
    /// environment chain
    /// @param constraintTemplate Constraint template for which to get an
    /// instantiation
    /// @param typeArgs The type args of the instantiation to get
    /// @return The found instantiation or nullptr if no instantiation was found
    IRConstraintInstantiation* findConstraintInstantiation(
        IRConstraintTemplate* constraintTemplate,
        std::vector<IRType*> const& typeArgs
    );

    /// @brief Get the std::unordered_multimap of constraint instantiations in
    /// this environment
    /// @return The std::unordered_multipmap of constraint instantiations in
    /// this environment
    auto& getConstraintInstantiationMap() { return m_constraintInstantiations; }

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

    /// @brief Get the std::unordered_multimap of struct instantiations in this
    /// module
    /// @return The std::unordered_multimap of struct instantiations in this
    /// module
    auto& getStructInstantiationMap() { return m_structInstantiations; }

    /// @brief Add a function with specified name and params to this environment
    /// @param function Function to add
    /// @return The added function or nullptr if a function with the same name
    /// and parameters already exists
    IRFunctionTemplate* addFunctionTemplate(IRFunctionTemplate* function);

    /// @brief Search for a function by name and params in this environment
    /// @param name Name of the function
    /// @param params Parameters of the function
    /// @return The found function or nullptr if the function was not found
    IRFunctionTemplate* getFunctionTemplate(
        std::string const& name, std::vector<IRType*> const& params
    );

    /// @brief Search for a function by name and params in the environment chain
    /// @param name Name of the function
    /// @param params Parameters of the function
    /// @return The found function or nullptr if the function was not found
    IRFunctionTemplate* findFunctionTemplate(
        std::string const& name, std::vector<IRType*> const& params
    );

    /// @brief Get the std::unordered_multimap of function templates in this
    /// environment
    auto& getFunctionTemplateMap() { return m_functions; }

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

    /// @brief Get the std::unordered_multimap of function instantiations in
    /// this environment
    auto& getFunctionInstantiationMap() { return m_functionInstantiations; }

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
    /// @brief Determine if @p actualType and @p genericType are compatible.
    /// @note Compatible is defined as either: \n
    /// @p actualType is equal to @p genericType; \n
    /// @p genericType is a generic placeholder type (e.g. T) and @p actualType
    /// is a concrete type (not a generic placeholder type) (e.g. i32) and @p
    /// typeArgs either does not yet contain a value for @p genericType or the
    /// the contained value is the same as @p actualType ; \n
    /// If @p allowGenericSubstitution == true:
    /// @p genericType is a generic placeholder type (e.g. T) and @p typeArgs
    /// either does not yet contain a value for @p genericType or the the
    /// contained value is the same as @p actualType ; \n
    /// If @p allowGenericSubstitution == false:
    /// @p genericType is a generic placeholder type (e.g. T) and @p actualType
    /// is a concrete type (not a generic placeholder type) (e.g. i32) and
    /// @p typeArgs either does not yet contain a value for @p genericType or
    /// the the contained value is the same as @p actualType ; \n
    /// @p actualType and @p genericType are both struct types and @p actualType
    /// is a concrete instantiation of @p genericType; \n
    /// @p actualType and @p genericType are both pointer/array types and their
    /// respective base types are compatible; \n
    /// @note If allowGenericSubtitution is false,
    /// @param genericType The template version of the type in question which
    /// may contain generic types, e.g.vec3<T>
    /// @param actualType The concrete version of the type in question from
    /// which type arguments are to be inferred, e.g. vec3<i32>
    /// @param typeArgs Map of type parameter values
    /// @param allowGenericSubstitution Treat @p actualType and @p genericType
    /// as compatible if both are generic types
    /// @return true if @p actualType is compatible with @p genericType
    /// , otherwise false
    bool inferTypeArgsAndMatch(
        IRType* actualType,
        IRType* genericType,
        std::unordered_map<IRGenericType*, IRType*>& typeArgs,
        bool allowGenericSubstitution
    );

    /// @brief Validate that @p actualType and @p genericType are compatible.
    /// @note Compatible is defined as either: \n
    /// @p actualType is equal to @p genericType; \n
    /// @p genericType is a generic placeholder type (e.g. T) and @p actualType
    /// is a concrete type (not a generic placeholder type) (e.g. i32) and @p
    /// typeArgs either does not yet contain a value for @p genericType or the
    /// the contained value is the same as @p actualType ; \n
    /// If @p allowGenericSubstitution == true:
    /// @p genericType is a generic placeholder type (e.g. T) and @p typeArgs
    /// either does not yet contain a value for @p genericType or the the
    /// contained value is the same as @p actualType ; \n
    /// If @p allowGenericSubstitution == false:
    /// @p genericType is a generic placeholder type (e.g. T) and @p actualType
    /// is a concrete type (not a generic placeholder type) (e.g. i32) and
    /// @p typeArgs either does not yet contain a value for @p genericType or
    /// the the contained value is the same as @p actualType ; \n
    /// @p actualType and @p genericType are both struct types and @p actualType
    /// is a concrete instantiation of @p genericType; \n
    /// @p actualType and @p genericType are both pointer/array types and their
    /// respective base types are compatible; \n
    /// @note If allowGenericSubtitution is false,
    /// @param genericType The template version of the type in question which
    /// may contain generic types, e.g.vec3<T>
    /// @param actualType The concrete version of the type in question from
    /// which type arguments are to be inferred, e.g. vec3<i32>
    /// @param typeArgs Map of type parameter values
    /// @param allowGenericSubstitution Treat @p actualType and @p genericType
    /// as compatible if both are generic types
    /// @return std::nullopt if @p actualType is compatible with @p genericType
    /// , otherwise a string describing why @p actualType is not compatible with
    /// @p genericType
    std::optional<std::string> inferTypeArgsAndValidate(
        IRType* genericType,
        IRType* actualType,
        std::unordered_map<IRGenericType*, IRType*>& typeArgs,
        bool allowGenericSubstitution
    );
};