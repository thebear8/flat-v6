#include "compiler.hpp"

CompilationContext::CompilationContext(CompilationOptions const& options, std::ostream& logStream) :
	options(options),
	logger(options.moduleSource, logStream),
	astCtx(),
	typeCtx(options.bits),
	llvmCtx(),
	module(options.moduleName, llvmCtx),
	target(nullptr),
	targetMachine(nullptr)
{
	typeCtx.builtinTypes.try_emplace("void", new VoidType(typeCtx));
	typeCtx.builtinTypes.try_emplace("bool", new BoolType(typeCtx));
	typeCtx.builtinTypes.try_emplace("u8", new IntegerType(typeCtx, false, 8));
	typeCtx.builtinTypes.try_emplace("u16", new IntegerType(typeCtx, false, 16));
	typeCtx.builtinTypes.try_emplace("u32", new IntegerType(typeCtx, false, 32));
	typeCtx.builtinTypes.try_emplace("u64", new IntegerType(typeCtx, false, 64));
	typeCtx.builtinTypes.try_emplace("i8", new IntegerType(typeCtx, true, 8));
	typeCtx.builtinTypes.try_emplace("i16", new IntegerType(typeCtx, true, 16));
	typeCtx.builtinTypes.try_emplace("i32", new IntegerType(typeCtx, true, 32));
	typeCtx.builtinTypes.try_emplace("i64", new IntegerType(typeCtx, true, 64));
	typeCtx.builtinTypes.try_emplace("char", new CharType(typeCtx, 32));
	typeCtx.builtinTypes.try_emplace("str", new StringType(typeCtx));

	llvm::InitializeAllTargetInfos();
	llvm::InitializeAllTargets();
	llvm::InitializeAllTargetMCs();
	llvm::InitializeAllAsmParsers();
	llvm::InitializeAllAsmPrinters();

	std::string error;
	target = llvm::TargetRegistry::lookupTarget(options.targetDesc, error);
	if (!target)
		throw std::exception(error.c_str());

	auto targetOptions = llvm::TargetOptions();
	auto relocModel = llvm::Optional<llvm::Reloc::Model>();
	targetMachine = target->createTargetMachine(options.targetDesc, options.cpuDesc, options.featureDesc, targetOptions, relocModel);
	if (!targetMachine)
		throw std::exception("Can't create TargetMachine");

	module.setDataLayout(targetMachine->createDataLayout());
	module.setTargetTriple(options.targetDesc);
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
		throw std::exception("TargetMachine can't emit a file of this type");

	passManager.run(module);
	output.flush();
}