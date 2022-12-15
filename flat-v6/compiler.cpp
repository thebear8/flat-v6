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

#include "util/string_switch.hpp"

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

CompilationContext::~CompilationContext()
{
	for (auto const& entry : modules)
		delete entry.second;
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

ModuleContext* CompilationContext::getModule(std::string const& name)
{
	if (!modules.contains(name))
		modules.try_emplace(name, new ModuleContext(*this, name));
	return modules.at(name);
}

Type* ModuleContext::getBuiltinOrStructType(std::string const& name)
{
	auto builtin = compCtx.getBuiltinType(name);
	if (builtin)
		return builtin;

	if (structTypes.contains(name))
		return structTypes.at(name);

	for (auto const& importName : imports)
	{
		auto mod = compCtx.getModule(importName);
		if (mod->structTypes.contains(name))
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
}

StructType* ModuleContext::createStruct(std::string const& name)
{
	if (structTypes.contains(name))
		return nullptr;

	structTypes.try_emplace(name, new StructType(compCtx.typeCtx, name));
	return structTypes.at(name);
}

StructType* ModuleContext::getStruct(std::string const& name)
{
	if (!structTypes.contains(name))
		structTypes.try_emplace(name, new StructType(compCtx.typeCtx, name));
	return structTypes.at(name);
}

IRFunctionDeclaration* ModuleContext::addFunction(IRFunctionDeclaration* function)
{
	if (functionDeclarations.contains(name))
		functionDeclarations.try_emplace(name, std::vector<IRFunctionDeclaration*>());
	auto& collection = functionDeclarations.at(name);

	for (auto candidate : collection)
	{
		if (function->params.size() == candidate->params.size()
			&& std::equal(function->params.begin(), function->params.end(), candidate->params.begin()))
			return nullptr;
	}

	collection.push_back(function);
	return function;
}

IRFunctionDeclaration* ModuleContext::getFunction(std::string const& name, std::vector<Type*> const& params)
{
	if (!functionDeclarations.contains(name))
		functionDeclarations.try_emplace(name, std::vector<IRFunctionDeclaration*>());
	auto& collection = functionDeclarations.at(name);

	for (auto candidate : collection)
	{
		if (params.size() == candidate->params.size()
			&& std::equal(params.begin(), params.end(), candidate->params.begin()))
			return candidate;
	}

	return nullptr;
}