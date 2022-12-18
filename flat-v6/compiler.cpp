#include "compiler.hpp"

#include <filesystem>
#include <fstream>

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/Host.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Analysis/LoopAnalysisManager.h>
#include <llvm/Analysis/CGSCCPassManager.h>

#include "data/ast.hpp"
#include "type/type.hpp"
#include "parser/parser.hpp"
#include "passes/struct_extraction_pass.hpp"
#include "passes/struct_population_pass.hpp"
#include "passes/function_extraction_pass.hpp"
#include "passes/ir_pass.hpp"
#include "passes/semantic_pass.hpp"
#include "passes/lowering_pass.hpp"
#include "passes/codegen_pass.hpp"

#include "util/string_switch.hpp"

CompilationContext::CompilationContext(CompilationOptions const& options, std::ostream& logStream) :
	options(options),
	logger(options.moduleSource, logStream),
	astCtx(),
	typeCtx(),
	llvmCtx(),
	llvmMod(options.moduleName, llvmCtx),
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

	llvmMod.setDataLayout(targetMachine->createDataLayout());
	llvmMod.setTargetTriple(options.targetDesc.targetTriple);

	typeCtx.setPointerSize(llvmMod.getDataLayout().getPointerSizeInBits());
}

CompilationContext::~CompilationContext()
{
	for (auto const& [name, mod] : modules)
		delete mod;
}

void CompilationContext::compile(std::string const& sourceDir, llvm::raw_pwrite_stream& output)
{
	std::vector<ASTSourceFile*> astSourceFiles;

	for (auto const& entry : std::filesystem::recursive_directory_iterator(sourceDir))
	{
		if (!entry.is_regular_file() || entry.path().extension() != ".fl")
			continue;

		std::ifstream inputStream(entry.path());
		std::string input(std::istreambuf_iterator<char>(inputStream), {});

		Parser parser(logger, astCtx, input);
		astSourceFiles.push_back(parser.sourceFile());
	}

	for (auto sf : astSourceFiles)
		StructExtractionPass(logger, *this, *getModule(sf->modulePath)).process(sf);

	for (auto sf : astSourceFiles)
		StructPopulationPass(logger, *this, *getModule(sf->modulePath)).process(sf);

	std::vector<IRSourceFile*> irSourceFiles;

	for (auto sf : astSourceFiles)
		irSourceFiles.push_back(
			IRPass(logger, *this, *getModule(sf->modulePath), getModule(sf->modulePath)->irCtx).process(sf)
		);

	for (auto sf : irSourceFiles)
		FunctionExtractionPass(logger, *this, *getModule(sf->path)).process(sf);

	for (auto sf : irSourceFiles)
		SemanticPass(logger, *this, *getModule(sf->path)).analyze(sf);

	for (auto sf : irSourceFiles)
		OperatorLoweringPass(logger, *this, *getModule(sf->path), getModule(sf->path)->irCtx).process(sf);

	for (auto sf : irSourceFiles)
		LLVMCodegenPass(logger, *this, *getModule(sf->path), llvmCtx, llvmMod).process(sf);

	llvmMod.print(llvm::outs(), nullptr);

	llvm::LoopAnalysisManager lam;
	llvm::FunctionAnalysisManager fam;
	llvm::CGSCCAnalysisManager cgam;
	llvm::ModuleAnalysisManager mam;

	llvm::PassBuilder pb;
	pb.registerModuleAnalyses(mam);
	pb.registerCGSCCAnalyses(cgam);
	pb.registerFunctionAnalyses(fam);
	pb.registerLoopAnalyses(lam);
	pb.crossRegisterProxies(lam, fam, cgam, mam);

	auto mpm = pb.buildPerModuleDefaultPipeline(llvm::OptimizationLevel::O1);
	mpm.run(llvmMod, mam);

	llvmMod.print(llvm::outs(), nullptr);

	llvm::legacy::PassManager passManager;
	if (targetMachine->addPassesToEmitFile(passManager, output, nullptr, llvm::CodeGenFileType::CGFT_ObjectFile))
		logger.error("TargetMachine cannot emit object files");

	passManager.run(llvmMod);
	output.flush();
}

ModuleContext* CompilationContext::getModule(std::string const& name)
{
	if (!modules.contains(name))
		modules.try_emplace(name, new ModuleContext(*this, name));
	return modules.at(name);
}

llvm::Function* CompilationContext::addLLVMFunction(IRFunctionDeclaration* function, llvm::Function* llvmFunction)
{
	if (llvmFunctions.contains(function))
		return nullptr;
	llvmFunctions.try_emplace(function, llvmFunction);
	return llvmFunctions.at(function);
}

llvm::Function* CompilationContext::getLLVMFunction(IRFunctionDeclaration* function)
{
	if (!llvmFunctions.contains(function))
		return nullptr;
	return llvmFunctions.at(function);
}

Type* CompilationContext::getBuiltinType(std::string const& name)
{
	return StringSwitch<Type*>(name)
		.Case("void", typeCtx.getVoid())
		.Case("bool", typeCtx.getBool())
		.Case("i8", typeCtx.getI8())
		.Case("i16", typeCtx.getI16())
		.Case("i32", typeCtx.getI32())
		.Case("i64", typeCtx.getI64())
		.Case("u8", typeCtx.getU8())
		.Case("u16", typeCtx.getU16())
		.Case("u32", typeCtx.getU32())
		.Case("u64", typeCtx.getU64())
		.Case("char", typeCtx.getChar())
		.Case("string", typeCtx.getString())
		.Default(nullptr);
}

IntegerType* CompilationContext::getIntegerType(size_t width, bool isSigned)
{
	return typeCtx.getIntegerType(width, isSigned);
}

PointerType* CompilationContext::getPointerType(Type* base)
{
	return typeCtx.getPointerType(base);
}

ArrayType* CompilationContext::getArrayType(Type* base)
{
	return typeCtx.getArrayType(base);
}

ModuleContext::~ModuleContext()
{
	for (auto const& [structName, structType] : structTypes)
		delete structType;
}

Type* ModuleContext::getBuiltinOrStructType(std::string const& typeName)
{
	auto builtin = compCtx.getBuiltinType(typeName);
	if (builtin)
		return builtin;

	if (structTypes.contains(typeName))
		return structTypes.at(typeName);

	for (auto const& importName : imports)
	{
		auto mod = compCtx.getModule(importName);
		if (mod->structTypes.contains(typeName))
			return mod->structTypes.at(name);
	}

	return nullptr;
}

Type* ModuleContext::getType(ASTType* type)
{
	assert(type && "Type cannot be null");

	if (auto namedType = dynamic_cast<ASTNamedType*>(type); namedType)
		return getBuiltinOrStructType(namedType->name);
	else if (auto pointerType = dynamic_cast<ASTPointerType*>(type); pointerType)
		return compCtx.getPointerType(getType(pointerType->base));
	else if (auto arrayType = dynamic_cast<ASTArrayType*>(type); arrayType)
		return compCtx.getArrayType(getType(arrayType->base));

	assert("Invalid ASTType*");
	return nullptr;
}

StructType* ModuleContext::createStruct(std::string const& structName)
{
	if (structTypes.contains(structName))
		return nullptr;

	structTypes.try_emplace(structName, new StructType(compCtx.typeCtx, structName));
	return structTypes.at(structName);
}

StructType* ModuleContext::getStruct(std::string const& structName)
{
	if (!structTypes.contains(structName))
		structTypes.try_emplace(structName, new StructType(compCtx.typeCtx, structName));
	return structTypes.at(structName);
}

StructType* ModuleContext::resolveStruct(std::string const& structName)
{
	if (auto structType = getStruct(structName))
		return structType;

	for (auto const& imp : imports)
	{
		if (auto structType = compCtx.getModule(imp)->getStruct(structName))
			return structType;
	}

	return nullptr;
}

IRFunctionDeclaration* ModuleContext::addFunction(IRFunctionDeclaration* function)
{
	if (!functionDeclarations.contains(function->name))
		functionDeclarations.try_emplace(function->name, std::vector<IRFunctionDeclaration*>());
	auto& collection = functionDeclarations.at(function->name);

	for (auto candidate : collection)
	{
		if (function->params.size() == candidate->params.size()
			&& std::equal(function->params.begin(), function->params.end(), candidate->params.begin()))
			return nullptr;
	}

	collection.push_back(function);
	return function;
}

IRFunctionDeclaration* ModuleContext::getFunction(std::string const& functionName, std::vector<Type*> const& params)
{
	if (!functionDeclarations.contains(functionName))
		functionDeclarations.try_emplace(functionName, std::vector<IRFunctionDeclaration*>());
	auto& collection = functionDeclarations.at(functionName);

	for (auto candidate : collection)
	{
		if (params.size() != candidate->params.size())
			continue;

		for (size_t i = 0; true; i++)
		{
			if (i == params.size())
				return candidate;

			if (params[i] != candidate->params[i].second)
				break;
		}
	}

	return nullptr;
}

IRFunctionDeclaration* ModuleContext::resolveFunction(std::string const& functionName, std::vector<Type*> const& params)
{
	if (auto function = getFunction(functionName, params))
		return function;

	for (auto const& imp : imports)
	{
		if (auto function = compCtx.getModule(imp)->getFunction(functionName, params))
			return function;
	}

	return nullptr;
}