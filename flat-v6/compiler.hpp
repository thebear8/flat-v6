#pragma once

#include <iostream>

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/Host.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/IR/LegacyPassManager.h>

#include "data/ast.hpp"
#include "type/type.hpp"
#include "parser/parser.hpp"
#include "passes/semantic_pass.hpp"
#include "passes/lowering_pass.hpp"
#include "passes/codegen_pass.hpp"

struct CompilationOptions
{
	std::string moduleName;
	std::string_view moduleSource;
	std::size_t bits;
	std::string targetDesc;
	std::string cpuDesc;
	std::string featureDesc;
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
	void compile(llvm::raw_pwrite_stream& output);
};