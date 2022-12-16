#pragma once

#include <iostream>
#include <llvm/IR/LLVMContext.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Target/TargetMachine.h>

#include "util/error_logger.hpp"
#include "util/graph_context.hpp"
#include "data/ast.hpp"
#include "data/ir.hpp"
#include "type/type.hpp"

struct TargetDescriptor
{
	std::string targetTriple;
	std::string cpuDesc;
	std::string featureDesc;

	TargetDescriptor() { }

	TargetDescriptor(std::string const& targetTriple, std::string const& cpuDesc, std::string const& featureDesc) :
		targetTriple(targetTriple), cpuDesc(cpuDesc), featureDesc(featureDesc) { }
};

struct CompilationOptions
{
	std::string moduleName;
	std::string_view moduleSource;
	TargetDescriptor targetDesc;
};

class ModuleContext;
class CompilationContext
{
	friend class ModuleContext;

private:
	CompilationOptions options;

	ErrorLogger logger;
	AstContext astCtx;
	TypeContext typeCtx;

	std::unordered_map<std::string, ModuleContext*> modules;

	llvm::LLVMContext llvmCtx;
	llvm::Module module;
	llvm::Target const * target;
	llvm::TargetMachine* targetMachine;
	std::unordered_map<IRFunctionDeclaration*, llvm::Function*> llvmFunctions;

public:
	CompilationContext(CompilationOptions const& options, std::ostream& logStream = std::cout);
	~CompilationContext();

public:
	void compile(std::string const& sourceDir);
	void compile(llvm::raw_pwrite_stream& output);

public:

	/// @brief Get or create a ModuleContext for the specified module
	/// @param name The module name
	/// @return Retrieved or created ModuleContext
	ModuleContext* getModule(std::string const& name);

	/// @brief Add an llvm::Function for an IR function. This doesn't actually belong in CompilationContext, but is here for now for lack of a better place.
	/// @param function The IR function to add an llvm::Function for
	/// @param llvmFunction The llvm::Function to add
	/// @return The added llvm::Function or nullptr if the function already exists
	llvm::Function* addLLVMFunction(IRFunctionDeclaration* function, llvm::Function* llvmFunction);

	/// @brief Get an llvm::Function for an IR function. This doesn't actually belong in CompilationContext, but is here for now for lack of a better place.
	/// @param function The IR function to get an llvm::Function for
	/// @return The retrieved llvm::Function or nullptr on failure
	llvm::Function* getLLVMFunction(IRFunctionDeclaration* function);

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
	VoidType* getVoid() { return typeCtx.getVoid(); }

	/// @brief Get bool type
	/// @return Type representing bool
	BoolType* getBool() { return typeCtx.getBool(); }

	/// @brief Get i8 type
	/// @return IntegerType representing i8
	IntegerType* getI8() { return typeCtx.getI8(); }

	/// @brief Get u8 type
	/// @return IntegerType representing u8
	IntegerType* getU8() { return typeCtx.getU8(); }

	/// @brief Get i16 type
	/// @return IntegerType representing i16
	IntegerType* getI16() { return typeCtx.getI16(); }

	/// @brief Get u16 type
	/// @return IntegerType representing u16
	IntegerType* getU16() { return typeCtx.getU16(); }

	/// @brief Get i32 type
	/// @return IntegerType representing i32
	IntegerType* getI32() { return typeCtx.getI32(); }

	/// @brief Get u32 type
	/// @return IntegerType representing u32
	IntegerType* getU32() { return typeCtx.getU32(); }

	/// @brief Get i64 type
	/// @return IntegerType representing i64
	IntegerType* getI64() { return typeCtx.getI64(); }

	/// @brief Get u64 type
	/// @return IntegerType representing u64
	IntegerType* getU64() { return typeCtx.getU64(); }

	/// @brief Get char type
	/// @return Type representing char
	CharType* getChar() { return typeCtx.getChar(); }

	/// @brief Get string type
	/// @return Type representing string
	StringType* getString() { return typeCtx.getString(); }
};

class ModuleContext
{
public:
	CompilationContext& compCtx;
	AstContext astCtx;
	GraphContext irCtx;

	std::string name;
	std::set<std::string> imports;
	std::unordered_map<std::string, StructType*> structTypes;
	std::unordered_map<std::string, std::vector<IRFunctionDeclaration*>> functionDeclarations;

public:
	ModuleContext(CompilationContext& compCtx, std::string const& name) :
		compCtx(compCtx), name(name) { }

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
	/// @return The created struct type or nullptr if the struct type already exists
	StructType* createStruct(std::string const& name);

	/// @brief Get a struct type
	/// @param name Name of the struct type
	/// @return The retrieved struct type or nullptr if the struct type does not exist
	StructType* getStruct(std::string const& name);

	/// @brief Resolve a struct type with specified name in the context of the module. The module is searched first, then the imports.
	/// @param name Name of the struct type
	/// @return The retrieved struct type or nullptr if the struct type does not exist
	StructType* resolveStruct(std::string const& name);

	/// @brief Add a function with specified name and params to the module
	/// @param function The function to add
	/// @return The added function or nullptr if a function with the same name and parameters already exists
	IRFunctionDeclaration* addFunction(IRFunctionDeclaration* function);

	/// @brief Get a function with specified name and params
	/// @param name Name of the function
	/// @param params Parameters of the function
	/// @return The retrieved function or nullptr if the function does not exist
	IRFunctionDeclaration* getFunction(std::string const& name, std::vector<Type*> const& params);

	/// @brief Resolve a function with specified name and params in the context of the module. The module is searched first, then the imports.
	/// @param name Name of the function
	/// @param params Parameters of the function
	/// @return The resolved function or nullptr if the function does not exist
	IRFunctionDeclaration* resolveFunction(std::string const& name, std::vector<Type*> const& params);
};