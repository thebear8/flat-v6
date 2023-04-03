cmake_minimum_required(VERSION 3.0.0)
project(flat-v6 VERSION 0.1.0)

# Setup CLI11
include(FetchContent)
FetchContent_Declare(
  cli11
  GIT_REPOSITORY https://github.com/CLIUtils/CLI11
  GIT_TAG        v2.2.0
)

FetchContent_MakeAvailable(cli11)

# Find LLVM Package
find_package(LLVM 15 REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# Set C++ Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Setup LLVM
include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

# Configure FLC Target
add_executable(flc)
target_include_directories(flc PRIVATE flat-v6)
llvm_map_components_to_libnames(llvm_libs support core)
target_link_libraries(flc ${llvm_libs})
target_link_libraries(flc CLI11::CLI11)

target_sources(flc PRIVATE
    flat-v6/compiler.cpp
    flat-v6/environment.cpp
    flat-v6/flat-v6.cpp
    flat-v6/parser/lexer.cpp
    flat-v6/parser/parser.cpp
    flat-v6/passes/codegen_pass.cpp
    flat-v6/passes/constraint_extraction_pass.cpp
    flat-v6/passes/constraint_instantiation_fixup_pass.cpp
    flat-v6/passes/constraint_population_pass.cpp
    flat-v6/passes/function_extraction_pass.cpp
    flat-v6/passes/function_population_pass.cpp
    flat-v6/passes/generic_lowering_pass.cpp
    flat-v6/passes/ir_pass.cpp
    flat-v6/passes/module_extraction_pass.cpp
    flat-v6/passes/module_import_population_pass.cpp
    flat-v6/passes/operator_lowering_pass.cpp
    flat-v6/passes/semantic_pass.cpp
    flat-v6/passes/struct_extraction_pass.cpp
    flat-v6/passes/struct_instantiation_fixup_pass.cpp
    flat-v6/passes/struct_population_pass.cpp
    flat-v6/passes/support/ast_type_resolver.cpp
    flat-v6/passes/support/instantiator.cpp
    flat-v6/util/error_logger.cpp
)