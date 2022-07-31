#include "compiler.hpp"

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

CompilationContext::CompilationContext(CompilationOptions const& options, std::ostream& logStream) :
	options(options),
	logger(options.moduleSource, logStream),
	astCtx(),
	typeCtx(),
	llvmCtx(),
	module(options.moduleName, llvmCtx),
	target(nullptr),
	targetMachine(nullptr)
{
	llvm::InitializeAllTargetInfos();
	llvm::InitializeAllTargets();
	llvm::InitializeAllTargetMCs();
	llvm::InitializeAllAsmParsers();
	llvm::InitializeAllAsmPrinters();

	std::string error;
	target = llvm::TargetRegistry::lookupTarget(options.targetDesc.targetTriple, error);
	if (!target)
		logger.error(error);

	auto targetOptions = llvm::TargetOptions();
	auto relocModel = llvm::Optional<llvm::Reloc::Model>();
	targetMachine = target->createTargetMachine(options.targetDesc.targetTriple, options.targetDesc.cpuDesc, options.targetDesc.featureDesc, targetOptions, relocModel);
	if (!targetMachine)
		logger.error("Can't create TargetMachine");

	module.setDataLayout(targetMachine->createDataLayout());
	module.setTargetTriple(options.targetDesc.targetTriple);

	typeCtx.setPointerSize(module.getDataLayout().getPointerSizeInBits());
	typeCtx.addBuiltinType("void", new VoidType(typeCtx));
	typeCtx.addBuiltinType("bool", new BoolType(typeCtx));
	typeCtx.addBuiltinType("u8", new IntegerType(typeCtx, false, 8));
	typeCtx.addBuiltinType("u16", new IntegerType(typeCtx, false, 16));
	typeCtx.addBuiltinType("u32", new IntegerType(typeCtx, false, 32));
	typeCtx.addBuiltinType("u64", new IntegerType(typeCtx, false, 64));
	typeCtx.addBuiltinType("i8", new IntegerType(typeCtx, true, 8));
	typeCtx.addBuiltinType("i16", new IntegerType(typeCtx, true, 16));
	typeCtx.addBuiltinType("i32", new IntegerType(typeCtx, true, 32));
	typeCtx.addBuiltinType("i64", new IntegerType(typeCtx, true, 64));
	typeCtx.addBuiltinType("char", new CharType(typeCtx, 32));
	typeCtx.addBuiltinType("str", new StringType(typeCtx));
}

void CompilationContext::compile(std::string const& outputFile)
{
	std::error_code error;
	llvm::raw_fd_ostream ofs(outputFile, error);
	if (error)
		logger.error("Cannot open output file " + outputFile);

	compile(ofs);
	ofs.close();
}

void CompilationContext::compile(llvm::raw_pwrite_stream& output)
{
	Parser parser(logger, astCtx, typeCtx, options.moduleSource);
	auto ast = parser.module();

	SemanticPass semanticPass(logger, astCtx, typeCtx);
	semanticPass.analyze(ast);

	OperatorLoweringPass loweringPass(logger, astCtx, typeCtx);
	loweringPass.process(ast);

	LLVMCodegenPass codegenPass(logger, typeCtx, llvmCtx, module);
	codegenPass.compile(ast);
	codegenPass.optimize();

	llvm::legacy::PassManager passManager;
	if (targetMachine->addPassesToEmitFile(passManager, output, nullptr, llvm::CodeGenFileType::CGFT_ObjectFile))
		logger.error("TargetMachine cannot emit object files");

	passManager.run(module);
	output.flush();
}