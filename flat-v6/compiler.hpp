#pragma once

#include <llvm/IR/LLVMContext.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Target/TargetMachine.h>

#include <iostream>

#include "data/ast.hpp"
#include "data/ir.hpp"
#include "environment.hpp"
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
class CompilationContext : public Environment
{
    friend class ModuleContext;

private:
    TargetDescriptor targetDesc;
    llvm::LLVMContext llvmCtx;
    llvm::Module llvmMod;
    llvm::Target const* target;
    llvm::TargetMachine* targetMachine;

    std::unordered_map<std::string, ModuleContext*> modules;
    std::unordered_map<IRFunctionDeclaration*, llvm::Function*> llvmFunctions;

    std::unordered_map<size_t, IRIntegerType*> m_signedIntegerTypes;
    std::unordered_map<size_t, IRIntegerType*> m_unsignedIntegerTypes;
    std::unordered_map<IRType*, IRPointerType*> m_pointerTypes;
    std::unordered_map<IRType*, IRArrayType*> m_arrayTypes;

    IRVoidType* m_void;
    IRBoolType* m_bool;
    IRIntegerType* m_i8;
    IRIntegerType* m_i16;
    IRIntegerType* m_i32;
    IRIntegerType* m_i64;
    IRIntegerType* m_u8;
    IRIntegerType* m_u16;
    IRIntegerType* m_u32;
    IRIntegerType* m_u64;
    IRCharType* m_char;
    IRStringType* m_string;

public:
    CompilationContext(
        TargetDescriptor const& targetDesc,
        std::ostream& logStream = std::cout);
    ~CompilationContext();

public:
    void compile(std::string const& sourceDir, llvm::raw_pwrite_stream& output);

    IRType* getType(std::string const& name) override;

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
        IRFunctionDeclaration* function, llvm::Function* llvmFunction);

    /// @brief Get an llvm::Function for an IR function. This doesn't actually
    /// belong in CompilationContext, but is here for now for lack of a better
    /// place.
    /// @param function The IR function to get an llvm::Function for
    /// @return The retrieved llvm::Function or nullptr on failure
    llvm::Function* getLLVMFunction(IRFunctionDeclaration* function);

    /// @brief Lookup a builtin type
    /// @param name Type name
    /// @return Type representing the specified type or nullptr on failure
    IRType* getBuiltinType(std::string const& name);

    /// @brief Get or create an IntegerType
    /// @param width Width in bits
    /// @param isSigned True => Signed Integer, False => Unsigned Integer
    /// @return Retrieved or created IntegerType
    IRIntegerType* getIntegerType(size_t width, bool isSigned);

    /// @brief Get or create a PointerType
    /// @param base Type to point to
    /// @return Retrieved or created PointerType
    IRPointerType* getPointerType(IRType* base);

    /// @brief Get or create an ArrayType
    /// @param base Element type
    /// @return Retrieved or created ArrayType
    IRArrayType* getArrayType(IRType* base);

    /// @brief Get void type
    /// @return Type representing void
    IRVoidType* getVoid() { return m_void; }

    /// @brief Get bool type
    /// @return Type representing bool
    IRBoolType* getBool() { return m_bool; }

    /// @brief Get i8 type
    /// @return IntegerType representing i8
    IRIntegerType* getI8() { return m_i8; }

    /// @brief Get u8 type
    /// @return IntegerType representing u8
    IRIntegerType* getU8() { return m_u8; }

    /// @brief Get i16 type
    /// @return IntegerType representing i16
    IRIntegerType* getI16() { return m_i16; }

    /// @brief Get u16 type
    /// @return IntegerType representing u16
    IRIntegerType* getU16() { return m_u16; }

    /// @brief Get i32 type
    /// @return IntegerType representing i32
    IRIntegerType* getI32() { return m_i32; }

    /// @brief Get u32 type
    /// @return IntegerType representing u32
    IRIntegerType* getU32() { return m_u32; }

    /// @brief Get i64 type
    /// @return IntegerType representing i64
    IRIntegerType* getI64() { return m_i64; }

    /// @brief Get u64 type
    /// @return IntegerType representing u64
    IRIntegerType* getU64() { return m_u64; }

    /// @brief Get char type
    /// @return Type representing char
    IRCharType* getChar() { return m_char; }

    /// @brief Get string type
    /// @return Type representing string
    IRStringType* getString() { return m_string; }
};

class ModuleContext : public Environment
{
public:
    CompilationContext& compCtx;
    GraphContext astCtx;
    GraphContext irCtx;

    std::string name;
    std::set<std::string> imports;

public:
    ModuleContext(CompilationContext& compCtx, std::string const& name)
        : Environment(name, &compCtx), compCtx(compCtx), name(name)
    {
    }

public:
    auto const& getFunctionList() { return m_functions; }
};