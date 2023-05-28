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

#include "ast/ast.hpp"
#include "parser/parser.hpp"
#include "passes/analysis/semantic_pass.hpp"
#include "passes/codegen/codegen_pass.hpp"
#include "passes/extraction/constraint_extraction_pass.hpp"
#include "passes/extraction/function_extraction_pass.hpp"
#include "passes/extraction/module_extraction_pass.hpp"
#include "passes/extraction/struct_extraction_pass.hpp"
#include "passes/lowering/operator_lowering_pass.hpp"
#include "passes/population/constraint_population_pass.hpp"
#include "passes/population/function_population_pass.hpp"
#include "passes/population/module_import_population_pass.hpp"
#include "passes/population/struct_population_pass.hpp"
#include "passes/support/ast_type_resolver.hpp"
#include "passes/support/call_target_resolver.hpp"
#include "passes/support/instantiator.hpp"
#include "passes/update/constraint_instantiation_update_pass.hpp"
#include "passes/update/function_instantiation_update_pass.hpp"
#include "passes/update/struct_instantiation_update_pass.hpp"
#include "support/formatter.hpp"
#include "util/string_switch.hpp"

CompilationContext::CompilationContext(std::ostream& logStream)
    : Environment("<global>", nullptr),
      m_logger(logStream, m_sourceFiles),
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
      m_string(new IRStringType())
{
    addBuiltinType("void", getVoid());
    addBuiltinType("bool", getBool());
    addBuiltinType("i8", getI8());
    addBuiltinType("i16", getI16());
    addBuiltinType("i32", getI32());
    addBuiltinType("i64", getI64());
    addBuiltinType("u8", getU8());
    addBuiltinType("u16", getU16());
    addBuiltinType("u32", getU32());
    addBuiltinType("u64", getU64());
    addBuiltinType("char", getChar());
    addBuiltinType("string", getString());

    m_signedIntegerTypes.try_emplace(8, m_i8);
    m_signedIntegerTypes.try_emplace(16, m_i16);
    m_signedIntegerTypes.try_emplace(32, m_i32);
    m_signedIntegerTypes.try_emplace(64, m_i64);
    m_unsignedIntegerTypes.try_emplace(8, m_u8);
    m_unsignedIntegerTypes.try_emplace(16, m_u16);
    m_unsignedIntegerTypes.try_emplace(32, m_u32);
    m_unsignedIntegerTypes.try_emplace(64, m_u64);
}

CompilationContext::~CompilationContext()
{
    for (auto [width, integerType] : m_signedIntegerTypes)
        delete integerType;

    for (auto [width, integerType] : m_unsignedIntegerTypes)
        delete integerType;

    for (auto const& [type, pointerType] : m_pointerTypes)
        delete pointerType;

    for (auto const& [type, arrayType] : m_arrayTypes)
        delete arrayType;

    delete m_void;
    delete m_bool;
    delete m_char;
    delete m_string;
}

void CompilationContext::readSourceFiles(std::string const& sourceDir)
{
    for (auto const& entry :
         std::filesystem::recursive_directory_iterator(sourceDir))
    {
        if (!entry.is_regular_file() || entry.path().extension() != ".fl")
            continue;

        std::ifstream inputStream(entry.path());
        std::string input(std::istreambuf_iterator<char>(inputStream), {});

        auto id = m_sourceFiles.size() + 1;
        m_sourceFiles.try_emplace(id, input);
    }
}

void CompilationContext::parseSourceFiles()
{
    for (auto& [id, source] : m_sourceFiles)
    {
        Parser parser(m_logger, m_astCtx, source, id);
        m_parsedSourceFiles.push_back(parser.sourceFile());
    }
}

void CompilationContext::runPasses()
{
    GraphContext envCtx;
    Formatter formatter;

    Instantiator instantiator(envCtx);
    CallTargetResolver callTargetResolver(instantiator);
    ASTTypeResolver astTypeResolver(instantiator);

    ModuleExtractionPass moduleExtractionPass(m_logger, *this, m_irCtx);
    ModuleImportPopulationPass moduleImportPopulationPass(m_logger, *this);

    StructExtractionPass structExtractionPass(m_logger, *this);
    ConstraintExtractionPass constraintExtractionPass(
        m_logger, *this, astTypeResolver
    );
    FunctionExtractionPass functionExtractionPass(
        m_logger, *this, envCtx, astTypeResolver
    );

    StructPopulationPass structPopulationPass(
        m_logger, *this, envCtx, astTypeResolver
    );
    ConstraintPopulationPass constraintPopulationPass(
        m_logger, *this, envCtx, astTypeResolver, instantiator
    );
    FunctionPopulationPass functionPopulationPass(
        m_logger, *this, envCtx, astTypeResolver, instantiator
    );

    SemanticPass semanticPass(
        m_logger, *this, envCtx, instantiator, callTargetResolver, formatter
    );

    StructInstantiationUpdatePass structInstantiationUpdatePass(
        envCtx, instantiator
    );
    ConstraintInstantiationUpdatePass constraintInstantiationUpdatePass(
        envCtx, instantiator
    );
    FunctionInstantiationUpdatePass functionInstantiationUpdatePass(
        envCtx, instantiator, callTargetResolver
    );

    OperatorLoweringPass operatorLoweringPass(m_logger, *this);

    for (auto sf : m_parsedSourceFiles)
        moduleExtractionPass.process(sf);

    for (auto sf : m_parsedSourceFiles)
        moduleImportPopulationPass.process(sf);

    for (auto sf : m_parsedSourceFiles)
        structExtractionPass.process(sf);

    for (auto sf : m_parsedSourceFiles)
        constraintExtractionPass.process(sf);

    for (auto sf : m_parsedSourceFiles)
        functionExtractionPass.process(sf);

    for (auto sf : m_parsedSourceFiles)
        structPopulationPass.process(sf);

    for (auto sf : m_parsedSourceFiles)
        constraintPopulationPass.process(sf);

    for (auto sf : m_parsedSourceFiles)
        functionPopulationPass.process(sf);

    for (auto const& [name, module] : m_modules)
        structInstantiationUpdatePass.process(module);

    for (auto const& [name, module] : m_modules)
        constraintInstantiationUpdatePass.process(module);

    for (auto const& [name, module] : m_modules)
        semanticPass.process(module);

    for (auto const& [name, module] : m_modules)
        functionInstantiationUpdatePass.process(module);

    for (auto const& [name, module] : m_modules)
        operatorLoweringPass.process(module);
}

void CompilationContext::generateCode(
    TargetDescriptor const& targetDesc, llvm::raw_pwrite_stream& output
)
{
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();

    std::string error;
    auto target =
        llvm::TargetRegistry::lookupTarget(targetDesc.targetTriple, error);
    if (!target)
        m_logger.fatal(error);

    auto targetOptions = llvm::TargetOptions();
    auto relocModel = llvm::Optional<llvm::Reloc::Model>();
    auto targetMachine = target->createTargetMachine(
        targetDesc.targetTriple,
        targetDesc.cpuDesc,
        targetDesc.featureDesc,
        targetOptions,
        relocModel
    );
    if (!targetMachine)
        m_logger.fatal("Can't create TargetMachine");

    llvm::LLVMContext llvmContext;
    llvm::Module llvmModule("<flat>", llvmContext);

    llvmModule.setDataLayout(targetMachine->createDataLayout());
    llvmModule.setTargetTriple(targetDesc.targetTriple);

    GraphContext envCtx;
    llvm::IRBuilder<> llvmBuilder(llvmContext);
    LLVMCodegenPass lcp(
        m_logger, *this, envCtx, llvmContext, llvmModule, llvmBuilder
    );

    for (auto [moduleName, irModule] : m_modules)
        lcp.process(irModule);

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
    mpm.run(llvmModule, mam);

    llvm::legacy::PassManager passManager;
    if (targetMachine->addPassesToEmitFile(
            passManager, output, nullptr, llvm::CodeGenFileType::CGFT_ObjectFile
        ))
    {
        m_logger.fatal("TargetMachine cannot emit object files");
    }

    passManager.run(llvmModule);
    output.flush();
}

IRModule* CompilationContext::addModule(IRModule* mod)
{
    if (m_modules.contains(mod->name))
        return nullptr;

    m_modules.try_emplace(mod->name, mod);
    return m_modules.at(mod->name);
}

IRModule* CompilationContext::getModule(std::string const& name)
{
    if (!m_modules.contains(name))
        return nullptr;
    return m_modules.at(name);
}

IRIntegerType* CompilationContext::getIntegerType(
    std::size_t width, bool isSigned
)
{
    if (isSigned)
    {
        if (!m_signedIntegerTypes.contains(width))
            m_signedIntegerTypes.try_emplace(
                width, new IRIntegerType(true, width)
            );
        return m_signedIntegerTypes.at(width);
    }
    else
    {
        if (!m_unsignedIntegerTypes.contains(width))
            m_unsignedIntegerTypes.try_emplace(
                width, new IRIntegerType(false, width)
            );
        return m_unsignedIntegerTypes.at(width);
    }
}

IRPointerType* CompilationContext::getPointerType(IRType* base)
{
    if (!m_pointerTypes.contains(base))
        m_pointerTypes.try_emplace(base, new IRPointerType(base));
    return m_pointerTypes.at(base);
}

IRArrayType* CompilationContext::getArrayType(IRType* base)
{
    if (!m_arrayTypes.contains(base))
        m_arrayTypes.try_emplace(base, new IRArrayType(base));
    return m_arrayTypes.at(base);
}

void CompilationContext::addUnaryOperator(
    std::string const& name, IRType* a, IRType* result
)
{
    FLC_ASSERT(addFunction(m_irCtx.make(IRUnaryIntrinsic(name, a, result))));
}

void CompilationContext::addBinaryOperator(
    std::string const& name, IRType* a, IRType* b, IRType* result
)
{
    FLC_ASSERT(addFunction(m_irCtx.make(IRBinaryIntrinsic(name, a, b, result)))
    );
}

void CompilationContext::addIntrinsicFunctions()
{
    auto intTypes = { getU8(), getU16(), getU32(), getU64(),
                      getI8(), getI16(), getI32(), getI64() };

    for (auto idx : intTypes)
    {
        auto t = m_irCtx.make(IRGenericType("T"));
        addFunction(m_irCtx.make(IRIndexIntrinsic(t, getArrayType(t), idx)));
    }

    for (auto a : intTypes)
    {
        addUnaryOperator("__pos__", a, a);
        addUnaryOperator("__neg__", a, a);
        addUnaryOperator("__not__", a, a);
    }

    addUnaryOperator("__lnot__", getBool(), getBool());

    for (auto a : intTypes)
    {
        for (auto b : intTypes)
        {
            auto result = getIntegerType(
                std::max(a->getBitSize(), b->getBitSize()), a->isSigned()
            );

            addBinaryOperator("__add__", a, b, result);
            addBinaryOperator("__sub__", a, b, result);
            addBinaryOperator("__mul__", a, b, result);
            addBinaryOperator("__div__", a, b, result);
            addBinaryOperator("__mod__", a, b, result);
        }
    }

    for (auto a : intTypes)
    {
        addBinaryOperator("__and__", a, a, a);
        addBinaryOperator("__or__", a, a, a);
        addBinaryOperator("__xor__", a, a, a);
        addBinaryOperator("__shl__", a, a, a);
        addBinaryOperator("__shr__", a, a, a);
    }

    addBinaryOperator("__land__", getBool(), getBool(), getBool());
    addBinaryOperator("__lor__", getBool(), getBool(), getBool());

    for (auto a : intTypes)
    {
        for (auto b : intTypes)
        {
            addBinaryOperator("__eq__", a, b, getBool());
            addBinaryOperator("__ne__", a, b, getBool());
        }
    }

    for (auto a : intTypes)
    {
        for (auto b : intTypes)
        {
            addBinaryOperator("__lt__", a, b, getBool());
            addBinaryOperator("__gt__", a, b, getBool());
            addBinaryOperator("__lteq__", a, b, getBool());
            addBinaryOperator("__gteq__", a, b, getBool());
        }
    }

    for (auto a : intTypes)
    {
        for (auto b : intTypes)
        {
            addBinaryOperator("__assign__", a, b, a);
        }
    }
}