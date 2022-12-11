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

class CompilationContext
{
private:
	CompilationOptions options;

	ErrorLogger logger;
	AstContext astCtx;
	TypeContext typeCtx;

	llvm::LLVMContext llvmCtx;
	llvm::Module module;
	llvm::Target const * target;
	llvm::TargetMachine* targetMachine;

public:
	CompilationContext(CompilationOptions const& options, std::ostream& logStream = std::cout);

public:
	void parse(std::vector<std::string> const& sources);
	void compile(std::string const& outputFile);
	void compile(llvm::raw_pwrite_stream& output);
};

class ModuleContext
{
private:
	AstContext astCtx;
	GraphContext irCtx;

	std::string path;
	std::vector<std::string> imports;
	std::unordered_map<std::string, StructType*> structTypes;
	std::unordered_map<std::string, std::vector<IRFunctionDeclaration*>> functionDeclarations;
};