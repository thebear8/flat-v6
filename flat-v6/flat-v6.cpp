#include <iostream>

#include <cli11/CLI11.hpp>

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

int main(int argc, char* argv[])
{
	std::string inputFile, outputFile;

	CLI::App app("flc");
	app.add_option("input, -i, --input", inputFile)->required()->check(CLI::ExistingFile);

	CLI11_PARSE(app, argc, argv);

	std::ifstream in{ inputFile };
	std::string input{ std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>() };

	TypeContext typeCtx(64);
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

	AstContext ctx;
	Parser parser(ctx, typeCtx, input, std::cout);
	auto program = parser.module();

	SemanticPass semPass(ctx, typeCtx, input, std::cout);
	semPass.analyze(program);

	OperatorLoweringPass opPass(ctx, typeCtx, input, std::cout);
	opPass.process(program);

	llvm::LLVMContext llvmCtx;

	llvm::Module mod("module", llvmCtx);
	LLVMCodegenPass llvmCodePass(typeCtx, llvmCtx, mod, input, std::cout);

	llvmCodePass.compile(program);

	std::cout << "\n====== BEFORE OPTIMIZATION ======\n\n";
	mod.print(llvm::outs(), nullptr);

	llvmCodePass.optimize();

	std::cout << "\n====== AFTER OPTIMIZATION ======\n\n";
	mod.print(llvm::outs(), nullptr);

	std::cout << "\n====== VERIFICATION ======\n\n";
	llvm::verifyModule(mod, &llvm::outs());

	std::error_code ec;
	llvm::raw_fd_ostream llvmOutput(inputFile + ".ll", ec);
	mod.print(llvmOutput, nullptr);

	std::string error;
	auto targetTriple = llvm::sys::getDefaultTargetTriple();

	llvm::InitializeAllTargetInfos();
	llvm::InitializeAllTargets();
	llvm::InitializeAllTargetMCs();
	llvm::InitializeAllAsmParsers();
	llvm::InitializeAllAsmPrinters();

	auto target = llvm::TargetRegistry::lookupTarget(targetTriple, error);
	if (!target)
	{
		std::cout << error;
		return 1;
	}

	auto relocationModel = llvm::Optional<llvm::Reloc::Model>();
	auto targetMachine = target->createTargetMachine(targetTriple, "generic", "", llvm::TargetOptions(), relocationModel);

	mod.setDataLayout(targetMachine->createDataLayout());
	mod.setTargetTriple(targetTriple);

	auto objectFile = llvm::raw_fd_ostream(inputFile + ".o", ec);
	if (ec)
	{
		std::cout << "Could not open output object file: " << ec.message() << "\n";
	}

	llvm::legacy::PassManager passManager;
	if (targetMachine->addPassesToEmitFile(passManager, objectFile, nullptr, llvm::CodeGenFileType::CGFT_ObjectFile))
	{
		std::cout << "TargetMachine can't emit a file of this type\n";
	}

	passManager.run(mod);
	objectFile.flush();
}