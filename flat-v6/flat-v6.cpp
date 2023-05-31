#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/TargetSelect.h>

#include <CLI/CLI.hpp>
#include <filesystem>
#include <iostream>

#include "compiler.hpp"
#include "parser/parser.hpp"
#include "util/string_switch.hpp"

int main(int argc, char* argv[])
{
    CLI::App app("flat compiler v6", "flat-v6");
    std::string sourceDir, output, target, cpuDesc, featureDesc;
    std::vector<std::string> additionalLibraryPaths;
    std::vector<std::string> additionalObjectFiles;
    bool optimize;

    app.add_option("source-dir, -s, --source-dir", sourceDir)
        ->type_name("DIRECTORY")
        ->description("Directory in which the source files are located")
        ->required()
        ->check([](std::string const& value) -> std::string {
            std::filesystem::directory_entry dir(value);
            if (!dir.exists() || !dir.is_directory())
                return "Cannot open source directory " + value;
            return "";
        });

    app.add_option("output, -o, --output", output)
        ->type_name("FILENAME")
        ->description("File to which the compiled executable is written to")
        ->check([](std::string const& value) -> std::string {
            std::ofstream ofs(value);
            if (ofs.fail() || ofs.bad() || !ofs.is_open())
                return "Cannot open output file " + value;
            ofs.close();
            return "";
        });

    app.add_option("lib, -L, --library", additionalLibraryPaths)
        ->type_name("DIRECTORY")
        ->description("Additional library paths to pass to the linker");

    app.add_option("lib, -l, --object", additionalObjectFiles)
        ->type_name("FILENAME")
        ->description("Additional object file names to pass to the linker");

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

    app.add_option("--optimize", optimize)
        ->default_val(false)
        ->description("Optimize generated LLVM IR");

    CLI11_PARSE(app, argc, argv);

    TargetDescriptor targetDesc = {};
    targetDesc.cpuDesc = cpuDesc;
    targetDesc.featureDesc = featureDesc;
    targetDesc.targetTriple = target;

    if (output.empty())
        output = std::filesystem::path(sourceDir).replace_extension(".out");

    std::error_code ec;
    auto objectFile = output + ".o";
    llvm::raw_fd_ostream objectStream(objectFile, ec);

    GraphContext astCtx, irCtx;
    CompilationContext ctx(astCtx, irCtx, std::cout);
    ctx.readSourceFiles(sourceDir);
    ctx.parseSourceFiles();
    ctx.runPasses();
    ctx.generateCode(targetDesc, objectStream, optimize);
    ctx.linkWithGCC(
        output, objectFile, additionalLibraryPaths, additionalObjectFiles
    );

    std::filesystem::remove(objectFile);
}