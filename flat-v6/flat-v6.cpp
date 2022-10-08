#include <iostream>
#include <filesystem>
#include <cli11/CLI11.hpp>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/Host.h>

#include "compiler.hpp"
#include "util/string_switch.hpp"
#include "parser/parser.hpp"
#include "passes/no_op_pass.hpp"

int main(int argc, char* argv[])
{
	CLI::App app("flat compiler v6", "flat-v6");
	std::string input, output, target, cpuDesc, featureDesc;

	app.add_option("input, -i, --input", input)->type_name("FILENAME")->required()->check([](std::string const& value) -> std::string {
		std::ifstream ifs(value);
		if (ifs.fail() || ifs.bad() || !ifs.is_open())
			return "Cannot open input file " + value;
		return "";
	});

	app.add_option("-o, --output", output)->type_name("FILENAME")->check([](std::string const& value) -> std::string {
		std::ofstream ofs(value);
		if (ofs.fail() || ofs.bad() || !ofs.is_open())
			return "Cannot open output file " + value;
		ofs.close();
		return "";
	});

	app.add_option("--target", target)->type_name("TARGET TRIPLE")->default_val(llvm::sys::getDefaultTargetTriple())->check([](std::string const& value) -> std::string
	{
		llvm::InitializeAllTargetInfos();
		llvm::InitializeAllTargets();
		llvm::InitializeAllTargetMCs();
		llvm::InitializeAllAsmParsers();
		llvm::InitializeAllAsmPrinters();

		std::string error;
		if (!llvm::TargetRegistry::lookupTarget(value, error))
			return value + " is not recognized as a valid target triple. This could be because llvm was not built with the matching backend.";
		return "";
	});

	app.add_option("--target-cpu", cpuDesc)->type_name("TARGET CPU")->default_val("generic")->check([](std::string const& value) -> std::string {
		return "";
	});

	app.add_option("--target-features", cpuDesc)->type_name("TARGET CPU FEATURES")->default_val("")->check([](std::string const& value) -> std::string {
		return "";
	});

	CLI11_PARSE(app, argc, argv);

	std::ifstream ifs(input);
	std::string source(std::istreambuf_iterator<char>(ifs), {});

	AstContext astCtx;
	TypeContext typeCtx;
	ErrorLogger logger(source, std::cout);

	Parser parser(logger, astCtx, typeCtx, source);
	auto ast = parser.module();

	NoOpPass noOpPass;
	auto begin = std::chrono::high_resolution_clock::now();
	for (int i = 0; i < 100000; i++)
		noOpPass.dispatch(ast);
	auto end = std::chrono::high_resolution_clock::now();

	std::cout << "Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - begin) << "\n";

	auto node = (AstNode*)ast;
	bool r = (typeid(*node) == typeid(int));

	/*
	CompilationOptions options = {};
	options.targetDesc.cpuDesc = cpuDesc;
	options.targetDesc.featureDesc = featureDesc;
	options.targetDesc.targetTriple = target;
	options.moduleName = std::filesystem::path(input).stem().string();
	options.moduleSource = source;

	CompilationContext ctx(options, std::cout);
	ctx.compile((output.empty() ? std::filesystem::path(input).replace_extension(".obj").string() : output));
	*/
}