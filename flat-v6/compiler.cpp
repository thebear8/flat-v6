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
#include <ranges>

#include "ast/ast.hpp"
#include "environment.hpp"
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
#include "util/graph_context.hpp"
#include "util/string_switch.hpp"

CompilationContext::CompilationContext(
    GraphContext& astCtx, GraphContext& irCtx, std::ostream& logStream
)
    : m_logger(logStream, m_sourceFiles), m_astCtx(astCtx), m_irCtx(irCtx)
{
    m_builtins = m_irCtx.make(IRModule("builtins", {}, {}, {}, {}));
    m_builtins->setIrCtx(m_irCtx.make(GraphContext()));
    m_builtins->setEnv(m_irCtx.make(Environment("builtins", nullptr)));

    addBuiltinType("void", (m_void = m_irCtx.make(IRVoidType())));
    addBuiltinType("bool", (m_bool = m_irCtx.make(IRBoolType())));
    addBuiltinType("u8", (m_u8 = m_irCtx.make(IRIntegerType(false, 8))));
    addBuiltinType("u16", (m_u16 = m_irCtx.make(IRIntegerType(false, 16))));
    addBuiltinType("u32", (m_u32 = m_irCtx.make(IRIntegerType(false, 32))));
    addBuiltinType("u64", (m_u64 = m_irCtx.make(IRIntegerType(false, 64))));
    addBuiltinType("i8", (m_i8 = m_irCtx.make(IRIntegerType(true, 8))));
    addBuiltinType("i16", (m_i16 = m_irCtx.make(IRIntegerType(true, 16))));
    addBuiltinType("i32", (m_i32 = m_irCtx.make(IRIntegerType(true, 32))));
    addBuiltinType("i64", (m_i64 = m_irCtx.make(IRIntegerType(true, 64))));
    addBuiltinType("char", (m_char = m_irCtx.make(IRCharType())));
    addBuiltinType("string", (m_string = m_irCtx.make(IRStringType())));

    auto integers = { getU8(), getU16(), getU32(), getU64(),
                      getI8(), getI16(), getI32(), getI64() };

    for (auto idx : integers)
    {
        auto t = m_irCtx.make(IRGenericType("T"));

        addBuiltinFunction(m_irCtx.make(IRIntrinsicFunction(
            m_builtins,
            "__index__",
            { t },
            { { "array", getArrayType(t) }, { "index", idx } },
            t,
            {}
        )));
    }

    for (auto a : integers)
    {
        addUnaryOperator("__pos__", a, a);
        addUnaryOperator("__neg__", a, a);
        addUnaryOperator("__not__", a, a);
    }

    addUnaryOperator("__lnot__", getBool(), getBool());

    for (auto a : integers)
    {
        for (auto b : integers)
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

    for (auto a : integers)
    {
        addBinaryOperator("__and__", a, a, a);
        addBinaryOperator("__or__", a, a, a);
        addBinaryOperator("__xor__", a, a, a);
        addBinaryOperator("__shl__", a, a, a);
        addBinaryOperator("__shr__", a, a, a);
    }

    addBinaryOperator("__land__", getBool(), getBool(), getBool());
    addBinaryOperator("__lor__", getBool(), getBool(), getBool());

    for (auto a : integers)
    {
        for (auto b : integers)
        {
            addBinaryOperator("__eq__", a, b, getBool());
            addBinaryOperator("__ne__", a, b, getBool());
        }
    }

    for (auto a : integers)
    {
        for (auto b : integers)
        {
            addBinaryOperator("__lt__", a, b, getBool());
            addBinaryOperator("__gt__", a, b, getBool());
            addBinaryOperator("__lteq__", a, b, getBool());
            addBinaryOperator("__gteq__", a, b, getBool());
        }
    }

    for (auto a : integers)
    {
        for (auto b : integers)
            addBinaryOperator("__assign__", a, b, a);
    }

    {
        auto t = m_irCtx.make(IRGenericType("T"));
        addBuiltinFunction(m_irCtx.make(IRIntrinsicFunction(
            m_builtins,
            "__u64_to_ptr",
            { t },
            { { "u", getU64() } },
            getPointerType(t),
            {}
        )));
    }

    {
        auto t = m_irCtx.make(IRGenericType("T"));
        addBuiltinFunction(m_irCtx.make(IRIntrinsicFunction(
            m_builtins,
            "__ptr_to_u64",
            { t },
            { { "p", getPointerType(t) } },
            getU64(),
            {}
        )));
    }

    {
        auto t = m_irCtx.make(IRGenericType("T"));
        addBuiltinFunction(m_irCtx.make(IRIntrinsicFunction(
            m_builtins,
            "__arr_to_ptr",
            { t },
            { { "a", getArrayType(t) } },
            getPointerType(t),
            {}
        )));
    }

    {
        addBuiltinFunction(m_irCtx.make(IRIntrinsicFunction(
            m_builtins,
            "__str_to_ptr",
            {},
            { { "s", getString() } },
            getPointerType(getU8()),
            {}
        )));
    }

    {
        addBuiltinFunction(m_irCtx.make(IRIntrinsicFunction(
            m_builtins, "__length", {}, { { "s", getString() } }, getU64(), {}
        )));
    }

    {
        auto t = m_irCtx.make(IRGenericType("T"));
        addBuiltinFunction(m_irCtx.make(IRIntrinsicFunction(
            m_builtins,
            "__length",
            { t },
            { { "s", getArrayType(t) } },
            getU64(),
            {}
        )));
    }
}

CompilationContext::~CompilationContext()
{
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

    OperatorLoweringPass operatorLoweringPass(m_logger, *this);

    StructInstantiationUpdatePass structInstantiationUpdatePass(
        envCtx, instantiator
    );

    SemanticPass semanticPass(
        m_logger,
        *this,
        envCtx,
        instantiator,
        callTargetResolver,
        formatter,
        structInstantiationUpdatePass
    );

    ConstraintInstantiationUpdatePass constraintInstantiationUpdatePass(
        envCtx, instantiator
    );
    FunctionInstantiationUpdatePass functionInstantiationUpdatePass(
        envCtx, instantiator, callTargetResolver
    );

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
        operatorLoweringPass.process(module);

    for (auto const& [name, module] : m_modules)
        semanticPass.process(module);

    for (auto const& [name, module] : m_modules)
        functionInstantiationUpdatePass.process(module);
}

void CompilationContext::generateCode(
    TargetDescriptor const& targetDesc,
    llvm::raw_pwrite_stream& output,
    bool optimize
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
    auto relocModel = llvm::Reloc::PIC_;
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
    if (optimize)
        mpm.run(llvmModule, mam);

    llvm::legacy::PassManager passManager;
    FLC_ASSERT(
        !targetMachine->addPassesToEmitFile(
            passManager, output, nullptr, llvm::CodeGenFileType::CGFT_ObjectFile
        ),
        "TargetMachine cannot emit object files"
    );

    passManager.run(llvmModule);
    llvmModule.dump();
    output.flush();
}

void CompilationContext::linkWithGCC(
    std::string const& executableFileName,
    std::string const& objectFileName,
    std::vector<std::string> const& additionalLibaryPaths,
    std::vector<std::string> const& additionalObjectNames
)
{
    std::string libString;
    for (auto const& lib : additionalLibaryPaths)
        libString += "-L\"" + lib + "\"";

    std::string objString;
    for (auto const& obj : additionalObjectNames)
        objString += "-l\"" + obj + "\"";

    auto command = "gcc -o " + executableFileName + " " + libString + " "
        + objString + objectFileName;

    std::cout << "Linker Command: " << command << "\n";
    std::system(command.c_str());
}

void CompilationContext::addBuiltinType(std::string const& name, IRType* type)
{
    FLC_ASSERT(type);

    FLC_ASSERT(m_builtins);
    FLC_ASSERT(m_builtins->getEnv());
    FLC_ASSERT(m_builtins->getEnv()->addBuiltinType(name, type));
}

void CompilationContext::addBuiltinFunction(IRFunction* function)
{
    FLC_ASSERT(function);

    FLC_ASSERT(m_builtins);
    FLC_ASSERT(m_builtins->getEnv());
    FLC_ASSERT(m_builtins->getEnv()->addFunction(function));
}

void CompilationContext::addUnaryOperator(
    std::string const& name, IRType* a, IRType* result
)
{
    FLC_ASSERT(a);
    FLC_ASSERT(result);

    FLC_ASSERT(m_builtins);
    FLC_ASSERT(m_builtins->getEnv());

    FLC_ASSERT(m_builtins->getEnv()->addFunction(m_irCtx.make(
        IRIntrinsicFunction(m_builtins, name, {}, { { "a", a } }, result, {})
    )));
}

void CompilationContext::addBinaryOperator(
    std::string const& name, IRType* a, IRType* b, IRType* result
)
{
    FLC_ASSERT(a);
    FLC_ASSERT(b);
    FLC_ASSERT(result);

    FLC_ASSERT(m_builtins);
    FLC_ASSERT(m_builtins->getEnv());

    FLC_ASSERT(
        m_builtins->getEnv()->addFunction(m_irCtx.make(IRIntrinsicFunction(
            m_builtins, name, {}, { { "a", a }, { "b", b } }, result, {}
        )))
    );
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
    if (width == 8)
        return (isSigned ? m_i8 : m_u8);
    else if (width == 16)
        return (isSigned ? m_i16 : m_u16);
    else if (width == 32)
        return (isSigned ? m_i32 : m_u32);
    else if (width == 64)
        return (isSigned ? m_i64 : m_u64);

    FLC_ASSERT(false);
    return nullptr;
}

IRPointerType* CompilationContext::getPointerType(IRType* base)
{
    if (!m_pointerTypes.contains(base))
        m_pointerTypes.try_emplace(base, m_irCtx.make(IRPointerType(base)));
    return m_pointerTypes.at(base);
}

IRArrayType* CompilationContext::getArrayType(IRType* base)
{
    if (!m_arrayTypes.contains(base))
        m_arrayTypes.try_emplace(base, m_irCtx.make(IRArrayType(base)));
    return m_arrayTypes.at(base);
}