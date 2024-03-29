#pragma once

#include <iostream>

#include "ast/ast.hpp"
#include "ir/ir.hpp"
#include "util/error_logger.hpp"

struct TargetDescriptor
{
    std::string targetTriple;
    std::string cpuDesc;
    std::string featureDesc;

    TargetDescriptor() {}

    TargetDescriptor(
        std::string const& targetTriple,
        std::string const& cpuDesc,
        std::string const& featureDesc
    )
        : targetTriple(targetTriple), cpuDesc(cpuDesc), featureDesc(featureDesc)
    {
    }
};

class GraphContext;
class IRModule;

namespace llvm
{
class raw_pwrite_stream;
}

class CompilationContext
{
private:
    ErrorLogger m_logger;
    GraphContext &m_astCtx, &m_irCtx;
    IRModule* m_builtins = nullptr;

    std::unordered_map<std::size_t, std::string> m_sourceFiles;
    std::vector<ASTSourceFile*> m_parsedSourceFiles;

    std::unordered_map<std::string, IRModule*> m_modules;
    std::unordered_map<IRType*, IRPointerType*> m_pointerTypes;
    std::unordered_map<IRType*, IRArrayType*> m_arrayTypes;

    IRVoidType* m_void = nullptr;
    IRBoolType* m_bool = nullptr;
    IRIntegerType* m_i8 = nullptr;
    IRIntegerType* m_i16 = nullptr;
    IRIntegerType* m_i32 = nullptr;
    IRIntegerType* m_i64 = nullptr;
    IRIntegerType* m_u8 = nullptr;
    IRIntegerType* m_u16 = nullptr;
    IRIntegerType* m_u32 = nullptr;
    IRIntegerType* m_u64 = nullptr;
    IRCharType* m_char = nullptr;
    IRStringType* m_string = nullptr;

public:
    CompilationContext(
        GraphContext& astCtx, GraphContext& irCtx, std::ostream& logStream
    );
    ~CompilationContext();

private:
    void addBuiltinType(std::string const& name, IRType* type);
    void addBuiltinFunction(IRFunction* function);
    void addUnaryOperator(std::string const& name, IRType* a, IRType* result);
    void addBinaryOperator(
        std::string const& name, IRType* a, IRType* b, IRType* result
    );

public:
    void readSourceFiles(std::string const& sourceDir);
    void parseSourceFiles();
    void runPasses();
    void generateCode(
        TargetDescriptor const& targetDesc,
        llvm::raw_pwrite_stream& output,
        bool optimize
    );
    void linkWithGCC(
        std::string const& executableFileName,
        std::string const& objectFileName,
        std::vector<std::string> const& libaryPaths,
        std::vector<std::string> const& objectNames
    );

public:
    /// @brief Add a module to this compilation context
    /// @param mod The module to add
    /// @return The added module or nullptr if a module with the same name
    /// already exists
    IRModule* addModule(IRModule* mod);

    /// @brief Search for a module by name in this compilation context
    /// @param name The name of the module to find
    /// @return The found module or nullptr if the module was not found
    IRModule* getModule(std::string const& name);

    /// @brief Get an IntegerType
    /// @param width Width in bits
    /// @param isSigned True => Signed Integer, False => Unsigned Integer
    /// @return Retrieved or created IntegerType
    IRIntegerType* getIntegerType(std::size_t width, bool isSigned);

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

    IRModule* getBuiltins() { return m_builtins; }
};