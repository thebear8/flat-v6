#pragma once
#include <string>

#include "../ir/ir.hpp"

class Formatter
{
public:
    std::string formatCallDescriptor(
        std::string targetName,
        std::vector<IRType*> const& typeArgs,
        std::vector<IRType*> const& args,
        IRType* result = nullptr
    );

    std::string formatFunctionDescriptor(IRFunction* value);

    std::string formatConstraintInstantiationDescriptor(
        IRConstraintInstantiation* value
    );

private:
    std::string joinTypeParams(std::vector<IRGenericType*> const& typeParams);
    std::string joinTypeParamsAndArgs(
        std::vector<IRGenericType*> const& typeParams,
        std::vector<IRType*> const& typeArgs
    );
};