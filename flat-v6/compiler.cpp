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
}

void CompilationContext::parse(std::vector<std::string> const& sources)
{

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
	auto ast = parser.sourceFile();

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