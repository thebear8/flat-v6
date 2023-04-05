#include "formatter.hpp"

#include <sstream>

#include "../../util/zip_view.hpp"

std::string Formatter::formatFunctionHeadDescriptor(IRFunctionHead* value)
{
    std::stringstream descriptor;
    descriptor << value->name;

    std::string params;
    for (auto param : value->params)
    {
        params += (params.empty() ? "" : ", ") + param.first + ": "
            + param.second->toString();
    }

    descriptor << "(" << params << ")";
    return descriptor.str();
}

std::string Formatter::formatFunctionTemplateDescriptor(
    IRFunctionTemplate* value
)
{
    std::stringstream descriptor;
    descriptor << value->name;

    std::string typeParams;
    for (auto typeParam : value->typeParams)
    {
        typeParams += (typeParams.empty() ? "" : ", ") + typeParam->toString();
    }

    if (!typeParams.empty())
        descriptor << "<" << typeParams << ">";

    std::string params;
    for (auto param : value->params)
    {
        params += (params.empty() ? "" : ", ") + param.first + ": "
            + param.second->toString();
    }

    descriptor << "(" << params << ")";
    return descriptor.str();
}

std::string Formatter::formatFunctionInstantiationDescriptor(
    IRFunctionInstantiation* value
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

    std::string params;
    for (auto param : value->params)
    {
        params += (params.empty() ? "" : ", ") + param.first + ": "
            + param.second->toString();
    }

    descriptor << "(" << params << ")";
    return descriptor.str();
}

std::string Formatter::formatCallDescriptor(
    std::string targetName,
    std::vector<IRType*> const& typeArgs,
    std::vector<IRType*> const& args
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

std::string Formatter::formatConstraintCondition(IRFunctionHead* value)
{
    std::stringstream descriptor;
    descriptor << value->name;

    std::string params;
    for (auto param : value->params)
    {
        params += (params.empty() ? "" : ", ") + param.first + ": "
            + param.second->toString();
    }

    descriptor << "(" << params << ")";
    return descriptor.str();
}