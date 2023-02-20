#pragma once
#include <string>
#include <vector>

#include "ast_node.hpp"

struct ASTSourceFile : public ASTNode
{
    std::string modulePath;
    std::vector<std::string> importPaths;
    std::vector<ASTDeclaration*> declarations;

    ASTSourceFile(
        SourceRef const& location,
        std::string const& modulePath,
        std::vector<std::string> const& importPaths,
        std::vector<ASTDeclaration*> const& declarations
    )
        : ASTNode(location),
          modulePath(modulePath),
          importPaths(importPaths),
          declarations(declarations)
    {
    }

    IMPLEMENT_ACCEPT()
};