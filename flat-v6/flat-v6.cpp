#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/TargetSelect.h>

#include <cli/CLI.hpp>
#include <filesystem>
#include <iostream>

#include "compiler.hpp"
#include "parser/parser.hpp"
#include "util/string_switch.hpp"

int main(int argc, char* argv[])
{
    CLI::App app("flat compiler v6", "flat-v6");
    std::string sourceDir, output, target, cpuDesc, featureDesc;

    app.add_option("source-dir, -s, --source-dir", sourceDir)
        ->type_name("DIRECTORY")
        ->required()
        ->check([](std::string const& value) -> std::string {
            std::filesystem::directory_entry dir(value);
            if (!dir.exists() || !dir.is_directory())
                return "Cannot open source directory " + value;
            return "";
        });

    app.add_option("-o, --output", output)
        ->type_name("FILENAME")
        ->check([](std::string const& value) -> std::string {
            std::ofstream ofs(value);
            if (ofs.fail() || ofs.bad() || !ofs.is_open())
                return "Cannot open output file " + value;
            ofs.close();
            return "";
        });

    app.add_option("--target", target)
        ->type_name("TARGET TRIPLE")
        ->default_val(llvm::sys::getDefaultTargetTriple())
        ->check(
            [](std::string const& value) -> std::
                                             string {
        llvm::InitializeAllTargetInfos();
        llvm::InitializeAllTargets();
        llvm::InitializeAllTargetMCs();
        llvm::InitializeAllAsmParsers();
        llvm::InitializeAllAsmPrinters();

        std::string error;
        if (!llvm::TargetRegistry::lookupTarget(value, error))
            return value
                + " is not recognized as a valid target triple. This could be because llvm was not built with the matching backend.";
        return "";
            });

    app.add_option("--target-cpu", cpuDesc)
        ->type_name("TARGET CPU")
        ->default_val("generic")
        ->check([](std::string const& value) -> std::string {
            return "";
        });

    app.add_option("--target-features", cpuDesc)
        ->type_name("TARGET CPU FEATURES")
        ->default_val("")
        ->check([](std::string const& value) -> std::string {
            return "";
        });

    CLI11_PARSE(app, argc, argv);

    TargetDescriptor targetDesc = {};
    targetDesc.cpuDesc = cpuDesc;
    targetDesc.featureDesc = featureDesc;
    targetDesc.targetTriple = target;

    auto outputName = output;
    if (outputName.empty())
        outputName =
            std::filesystem::path(sourceDir).replace_extension(".obj").string();

    std::error_code ec;
    llvm::raw_fd_ostream outStream(outputName, ec);
    CompilationContext ctx(std::cout);
    ctx.readSourceFiles(sourceDir);
    ctx.parseSourceFiles();
    ctx.runPasses();
    ctx.generateCode(targetDesc, outStream);
}