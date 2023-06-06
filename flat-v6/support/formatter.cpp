#include "formatter.hpp"

#include <sstream>

#include "../util/zip_view.hpp"

std::string Formatter::joinTypeParams(
    std::vector<IRGenericType*> const& typeParams
)
{
    std::string value;
    for (auto tp : typeParams)
        value += (value.empty() ? "" : ", ") + tp->toString();
    return value;
}

std::string Formatter::joinTypeParamsAndArgs(
    std::vector<IRGenericType*> const& typeParams,
    std::vector<IRType*> const& typeArgs
)
{
    std::string value;
    for (auto [tp, ta] : zip(typeParams, typeArgs))
        value += (value.empty() ? "" : ", ") + tp->toString() + " = "
            + ta->toString();
    return value;
}

std::string Formatter::formatFunctionDescriptor(IRFunction* value)
{
    std::stringstream descriptor;
    descriptor << value->name;

    if (value->typeParams.size() != 0
        && value->typeParams.size() == value->typeArgs.size())
        descriptor << "<"
                   << joinTypeParamsAndArgs(value->typeParams, value->typeArgs)
                   << ">";
    else if (value->typeParams.size() != 0)
        descriptor << "<" << joinTypeParams(value->typeParams) << ">";

    std::string params;
    for (auto param : value->params)
    {
        params += (params.empty() ? "" : ", ") + param.first + ": "
            + param.second->toString();
    }

    descriptor << "(" << params << ")";
    if (value->result)
        descriptor << ": " << value->result->toString();

    return descriptor.str();
}

std::string Formatter::formatCallDescriptor(
    std::string targetName,
    std::vector<IRType*> const& typeArgs,
    std::vector<IRType*> const& args,
    IRType* result
)
{
    std::stringstream descriptor;
    descriptor << targetName;

    std::string typeArgString;
    for (auto typeArg : typeArgs)
    {
        typeArgString +=
            (typeArgString.empty() ? "" : ", ") + typeArg->toString();
    }

    if (!typeArgString.empty())
        descriptor << "<" << typeArgString << ">";

    std::string argString;
    for (auto arg : args)
    {
        argString += (argString.empty() ? "" : ", ") + arg->toString();
    }

    descriptor << "(" << argString << ")";
    if (result)
        descriptor << ": " << result->toString();

    return descriptor.str();
}

std::string Formatter::formatConstraintInstantiationDescriptor(
    IRConstraintInstantiation* value
)
{
    std::stringstream descriptor;
    descriptor << value->name;

    std::string typeArgs;
    auto zippedTypeArgs = zip_view(
        std::views::all(value->getInstantiatedFrom()->typeParams),
        std::views::all(value->typeArgs)
    );
    for (auto [typeParam, typeArg] : zippedTypeArgs)
    {
        typeArgs += (typeArgs.empty() ? "" : ", ") + typeParam->toString()
            + " = " + typeArg->toString();
    }

    if (!typeArgs.empty())
        descriptor << "<" << typeArgs << ">";

    return descriptor.str();
}