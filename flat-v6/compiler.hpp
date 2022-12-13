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
public:
	CompilationOptions options;

	ErrorLogger logger;
	AstContext astCtx;
	TypeContext typeCtx;

	std::unordered_map<std::string, ModuleContext*> modules;

private:
	llvm::LLVMContext llvmCtx;
	llvm::Module module;
	llvm::Target const * target;
	llvm::TargetMachine* targetMachine;

public:
	CompilationContext(CompilationOptions const& options, std::ostream& logStream = std::cout);
	~CompilationContext();

public:
	void parse(std::vector<std::string> const& sources);
	void compile(std::string const& outputFile);
	void compile(llvm::raw_pwrite_stream& output);

public:

	/// @brief Get or create a ModuleContext for the specified module
	/// @param name The module name
	/// @return Retrieved or created ModuleContext
	ModuleContext* getModule(std::string const& name);

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

	/// @brief Lookup a builtin type or struct type
	/// @param name Name of the type to get
	/// @return The specified type on success or nullptr on failure
	Type* getBuiltinOrStructType(std::string const& name);

	/// @brief Get a Type from an ASTType in the context of the module
	/// @param type ASTType to get a Type for
	/// @return The Type that represents the given ASTType
	Type* getType(ASTType* type);

	/// @brief Get or create a struct type
	/// @param name The name of the struct type
	/// @return The retrieved or created StructType
	StructType* getStructType(std::string const& name);
};