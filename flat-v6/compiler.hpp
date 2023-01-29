#pragma once

#include <llvm/IR/LLVMContext.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Target/TargetMachine.h>

#include <iostream>

#include "data/ast.hpp"
#include "data/ir.hpp"
#include "data/type.hpp"
#include "util/error_logger.hpp"
#include "util/graph_context.hpp"

struct TargetDescriptor
{
    std::string targetTriple;
    std::string cpuDesc;
    std::string featureDesc;

    TargetDescriptor() {}

    TargetDescriptor(
        std::string const& targetTriple,
        std::string const& cpuDesc,
        std::string const& featureDesc)
        : targetTriple(targetTriple), cpuDesc(cpuDesc), featureDesc(featureDesc)
    {
    }
};

class ModuleContext;
class CompilationContext
{
    friend class ModuleContext;

private:
    TargetDescriptor targetDesc;
    llvm::LLVMContext llvmCtx;
    llvm::Module llvmMod;
    llvm::Target const* target;
    llvm::TargetMachine* targetMachine;

    std::unordered_map<std::string, ModuleContext*> modules;
    std::unordered_map<IRFunctionDefinition*, llvm::Function*> llvmFunctions;

    std::unordered_map<size_t, IntegerType*> m_signedIntegerTypes;
    std::unordered_map<size_t, IntegerType*> m_unsignedIntegerTypes;
    std::unordered_map<Type*, PointerType*> m_pointerTypes;
    std::unordered_map<Type*, ArrayType*> m_arrayTypes;

    VoidType* m_void;
    BoolType* m_bool;
    IntegerType* m_i8;
    IntegerType* m_i16;
    IntegerType* m_i32;
    IntegerType* m_i64;
    IntegerType* m_u8;
    IntegerType* m_u16;
    IntegerType* m_u32;
    IntegerType* m_u64;
    CharType* m_char;
    StringType* m_string;

public:
    CompilationContext(
        TargetDescriptor const& targetDesc,
        std::ostream& logStream = std::cout);
    ~CompilationContext();

public:
    void compile(std::string const& sourceDir, llvm::raw_pwrite_stream& output);

public:
    /// @brief Get or create a ModuleContext for the specified module
    /// @param name The module name
    /// @return Retrieved or created ModuleContext
    ModuleContext* getModule(std::string const& name);

    /// @brief Add an llvm::Function for an IR function. This doesn't actually
    /// belong in CompilationContext, but is here for now for lack of a better
    /// place.
    /// @param function The IR function to add an llvm::Function for
    /// @param llvmFunction The llvm::Function to add
    /// @return The added llvm::Function or nullptr if the function already
    /// exists
    llvm::Function* addLLVMFunction(
        IRFunctionDefinition* function, llvm::Function* llvmFunction);

    /// @brief Get an llvm::Function for an IR function. This doesn't actually
    /// belong in CompilationContext, but is here for now for lack of a better
    /// place.
    /// @param function The IR function to get an llvm::Function for
    /// @return The retrieved llvm::Function or nullptr on failure
    llvm::Function* getLLVMFunction(IRFunctionDefinition* function);

    /// @brief Lookup a builtin type
    /// @param name Type name
    /// @return Type representing the specified type or nullptr on failure
    Type* getBuiltinType(std::string const& name);

    /// @brief Get or create an IntegerType
    /// @param width Width in bits
    /// @param isSigned True => Signed Integer, False => Unsigned Integer
    /// @return Retrieved or created IntegerType
    IntegerType* getIntegerType(size_t width, bool isSigned);

    /// @brief Get or create a PointerType
    /// @param base Type to point to
    /// @return Retrieved or created PointerType
    PointerType* getPointerType(Type* base);

    /// @brief Get or create an ArrayType
    /// @param base Element type
    /// @return Retrieved or created ArrayType
    ArrayType* getArrayType(Type* base);

    /// @brief Get void type
    /// @return Type representing void
    VoidType* getVoid() { return m_void; }

    /// @brief Get bool type
    /// @return Type representing bool
    BoolType* getBool() { return m_bool; }

    /// @brief Get i8 type
    /// @return IntegerType representing i8
    IntegerType* getI8() { return m_i8; }

    /// @brief Get u8 type
    /// @return IntegerType representing u8
    IntegerType* getU8() { return m_u8; }

    /// @brief Get i16 type
    /// @return IntegerType representing i16
    IntegerType* getI16() { return m_i16; }

    /// @brief Get u16 type
    /// @return IntegerType representing u16
    IntegerType* getU16() { return m_u16; }

    /// @brief Get i32 type
    /// @return IntegerType representing i32
    IntegerType* getI32() { return m_i32; }

    /// @brief Get u32 type
    /// @return IntegerType representing u32
    IntegerType* getU32() { return m_u32; }

    /// @brief Get i64 type
    /// @return IntegerType representing i64
    IntegerType* getI64() { return m_i64; }

    /// @brief Get u64 type
    /// @return IntegerType representing u64
    IntegerType* getU64() { return m_u64; }

    /// @brief Get char type
    /// @return Type representing char
    CharType* getChar() { return m_char; }

    /// @brief Get string type
    /// @return Type representing string
    StringType* getString() { return m_string; }
};

class ModuleContext
{
public:
    CompilationContext& compCtx;
    GraphContext astCtx;
    GraphContext irCtx;

    std::string name;
    std::set<std::string> imports;
    std::unordered_map<std::string, StructType*> structTypes;
    std::unordered_map<std::string, std::vector<IRFunctionDefinition*>>
        functionDefinitions;

public:
    ModuleContext(CompilationContext& compCtx, std::string const& name)
        : compCtx(compCtx), name(name)
    {
    }

    ~ModuleContext();

    /// @brief Lookup a builtin type or struct type
    /// @param name Name of the type to get
    /// @return The specified type on success or nullptr on failure
    Type* getBuiltinOrStructType(std::string const& name);

    /// @brief Get a Type from an ASTType in the context of the module
    /// @param type ASTType to get a Type for
    /// @return The Type that represents the given ASTType
    Type* getType(ASTType* type);

    /// @brief Create a struct type with specified name
    /// @param name The name of the struct type
    /// @return The created struct type or nullptr if the struct type already
    /// exists
    StructType* createStruct(std::string const& name);

    /// @brief Get a struct type
    /// @param name Name of the struct type
    /// @return The retrieved struct type or nullptr if the struct type does not
    /// exist
    StructType* getStruct(std::string const& name);

    /// @brief Resolve a struct type with specified name in the context of the
    /// module. The module is searched first, then the imports.
    /// @param name Name of the struct type
    /// @return The retrieved struct type or nullptr if the struct type does not
    /// exist
    StructType* resolveStruct(std::string const& name);

    /// @brief Add a function with specified name and params to the module
    /// @param function The function to add
    /// @return The added function or nullptr if a function with the same name
    /// and parameters already exists
    IRFunctionDefinition* addFunction(IRFunctionDefinition* function);

    /// @brief Get a function with specified name and params
    /// @param name Name of the function
    /// @param params Parameters of the function
    /// @return The retrieved function or nullptr if the function does not exist
    IRFunctionDefinition* getFunction(
        std::string const& name, std::vector<Type*> const& params);

    /// @brief Resolve a function with specified name and params in the context
    /// of the module. The module is searched first, then the imports.
    /// @param name Name of the function
    /// @param params Parameters of the function
    /// @return The resolved function or nullptr if the function does not exist
    IRFunctionDefinition* resolveFunction(
        std::string const& name, std::vector<Type*> const& params);
};