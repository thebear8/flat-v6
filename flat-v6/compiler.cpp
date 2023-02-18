#include "compiler.hpp"

#include <llvm/Analysis/CGSCCPassManager.h>
#include <llvm/Analysis/LoopAnalysisManager.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Verifier.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>

#include <filesystem>
#include <fstream>

#include "data/ast.hpp"
#include "parser/parser.hpp"
#include "passes/codegen_pass.hpp"
#include "passes/function_extraction_pass.hpp"
#include "passes/ir_pass.hpp"
#include "passes/lowering_pass.hpp"
#include "passes/semantic_pass.hpp"
#include "passes/struct_extraction_pass.hpp"
#include "util/string_switch.hpp"

CompilationContext::CompilationContext(
    TargetDescriptor const& targetDesc, std::ostream& logStream
)
    : Environment("<global>", nullptr),
      targetDesc(targetDesc),
      llvmCtx(),
      llvmMod("flat", llvmCtx),
      target(nullptr),
      targetMachine(nullptr),
      m_void(new IRVoidType()),
      m_bool(new IRBoolType()),
      m_i8(new IRIntegerType(true, 8)),
      m_i16(new IRIntegerType(true, 16)),
      m_i32(new IRIntegerType(true, 32)),
      m_i64(new IRIntegerType(true, 64)),
      m_u8(new IRIntegerType(false, 8)),
      m_u16(new IRIntegerType(false, 16)),
      m_u32(new IRIntegerType(false, 32)),
      m_u64(new IRIntegerType(false, 64)),
      m_char(new IRCharType()),
      m_string(new IRStringType()) {
    m_signedIntegerTypes.try_emplace(8, m_i8);
    m_signedIntegerTypes.try_emplace(16, m_i16);
    m_signedIntegerTypes.try_emplace(32, m_i32);
    m_signedIntegerTypes.try_emplace(64, m_i64);
    m_unsignedIntegerTypes.try_emplace(8, m_u8);
    m_unsignedIntegerTypes.try_emplace(16, m_u16);
    m_unsignedIntegerTypes.try_emplace(32, m_u32);
    m_unsignedIntegerTypes.try_emplace(64, m_u64);

    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();

    std::string error;
    target = llvm::TargetRegistry::lookupTarget(targetDesc.targetTriple, error);
    if (!target)
        ErrorLogger(std::cout, {}).fatal(error);

    auto targetOptions = llvm::TargetOptions();
    auto relocModel = llvm::Optional<llvm::Reloc::Model>();
    targetMachine = target->createTargetMachine(
        targetDesc.targetTriple,
        targetDesc.cpuDesc,
        targetDesc.featureDesc,
        targetOptions,
        relocModel
    );
    if (!targetMachine)
        ErrorLogger(std::cout, {}).fatal("Can't create TargetMachine");

    llvmMod.setDataLayout(targetMachine->createDataLayout());
    llvmMod.setTargetTriple(targetDesc.targetTriple);
}

CompilationContext::~CompilationContext() {
    for (auto const& [name, mod] : modules)
        delete mod;
}

void CompilationContext::compile(
    std::string const& sourceDir, llvm::raw_pwrite_stream& output
) {
    GraphContext astCtx;
    std::vector<ASTSourceFile*> astSourceFiles;
    std::unordered_map<size_t, std::string> sources;
    ErrorLogger logger(std::cout, sources);

    for (auto const& entry :
         std::filesystem::recursive_directory_iterator(sourceDir)) {
        if (!entry.is_regular_file() || entry.path().extension() != ".fl")
            continue;

        std::ifstream inputStream(entry.path());
        std::string input(std::istreambuf_iterator<char>(inputStream), {});

        auto id = sources.size() + 1;
        sources.try_emplace(id, input);

        Parser parser(logger, astCtx, input, id);
        astSourceFiles.push_back(parser.sourceFile());
    }

    for (auto sf : astSourceFiles)
        StructExtractionPass(logger, *this, *getModule(sf->modulePath))
            .process(sf);

    std::vector<IRSourceFile*> irSourceFiles;

    for (auto sf : astSourceFiles)
        irSourceFiles.push_back(IRPass(
                                    logger,
                                    *this,
                                    *getModule(sf->modulePath),
                                    getModule(sf->modulePath)->irCtx
        )
                                    .process(sf));

    for (auto sf : irSourceFiles)
        FunctionExtractionPass(logger, *this, *getModule(sf->path)).process(sf);

    for (auto sf : irSourceFiles)
        SemanticPass(logger, *this, *getModule(sf->path)).analyze(sf);

    for (auto sf : irSourceFiles)
        OperatorLoweringPass(
            logger, *this, *getModule(sf->path), getModule(sf->path)->irCtx
        )
            .process(sf);

    for (auto sf : irSourceFiles)
        LLVMCodegenPass(logger, *this, *getModule(sf->path), llvmCtx, llvmMod)
            .process(sf);

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
    if (targetMachine->addPassesToEmitFile(
            passManager, output, nullptr, llvm::CodeGenFileType::CGFT_ObjectFile
        ))
        logger.fatal("TargetMachine cannot emit object files");

    passManager.run(llvmMod);
    output.flush();
}

IRType* CompilationContext::getType(std::string const& name) {
    if (auto b = getBuiltinType(name))
        return b;
    return Environment::getType(name);
}

ModuleContext* CompilationContext::getModule(std::string const& name) {
    if (!modules.contains(name))
        modules.try_emplace(name, new ModuleContext(*this, name));
    return modules.at(name);
}

llvm::Function* CompilationContext::addLLVMFunction(
    IRFunctionDeclaration* function, llvm::Function* llvmFunction
) {
    if (llvmFunctions.contains(function))
        return nullptr;
    llvmFunctions.try_emplace(function, llvmFunction);
    return llvmFunctions.at(function);
}

llvm::Function* CompilationContext::getLLVMFunction(
    IRFunctionDeclaration* function
) {
    if (!llvmFunctions.contains(function))
        return nullptr;
    return llvmFunctions.at(function);
}

IRType* CompilationContext::getBuiltinType(std::string const& name) {
    return StringSwitch<IRType*>(name)
        .Case("void", getVoid())
        .Case("bool", getBool())
        .Case("i8", getI8())
        .Case("i16", getI16())
        .Case("i32", getI32())
        .Case("i64", getI64())
        .Case("u8", getU8())
        .Case("u16", getU16())
        .Case("u32", getU32())
        .Case("u64", getU64())
        .Case("char", getChar())
        .Case("string", getString())
        .Default(nullptr);
}

IRIntegerType* CompilationContext::getIntegerType(size_t width, bool isSigned) {
    if (isSigned) {
        if (!m_signedIntegerTypes.contains(width))
            m_signedIntegerTypes.try_emplace(
                width, new IRIntegerType(true, width)
            );
        return m_signedIntegerTypes.at(width);
    } else {
        if (!m_unsignedIntegerTypes.contains(width))
            m_unsignedIntegerTypes.try_emplace(
                width, new IRIntegerType(false, width)
            );
        return m_unsignedIntegerTypes.at(width);
    }
}

IRPointerType* CompilationContext::getPointerType(IRType* base) {
    if (!m_pointerTypes.contains(base))
        m_pointerTypes.try_emplace(base, new IRPointerType(base));
    return m_pointerTypes.at(base);
}

IRArrayType* CompilationContext::getArrayType(IRType* base) {
    if (!m_arrayTypes.contains(base))
        m_arrayTypes.try_emplace(base, new IRArrayType(base));
    return m_arrayTypes.at(base);
}