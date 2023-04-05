#pragma once
#include <string>

#include "../../ir/ir.hpp"

class Formatter
{
public:
    std::string formatFunctionHeadDescriptor(IRFunctionHead* value);
    std::string formatFunctionTemplateDescriptor(IRFunctionTemplate* value);
    std::string formatFunctionInstantiationDescriptor(
        IRFunctionInstantiation* value
    );
    std::string formatCallDescriptor(
        std::string targetName,
        std::vector<IRType*> const& typeArgs,
        std::vector<IRType*> const& args
    );

    std::string formatConstraintInstantiationDescriptor(
        IRConstraintInstantiation* value
    );
    std::string formatConstraintCondition(IRFunctionHead* value);
};