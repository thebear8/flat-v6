#pragma once
#include <ostream>
#include <string>
#include <string_view>
#include <unordered_map>

#include "../data/source_ref.hpp"

class ErrorLogger
{
private:
    std::ostream& output;
    std::unordered_map<std::size_t, std::string> const& sources;

public:
    ErrorLogger(
        std::ostream& output,
        std::unordered_map<std::size_t, std::string> const& sources
    )
        : output(output), sources(sources)
    {
    }

public:
    [[noreturn]] void fatal(std::string const& message);
    [[noreturn]] void fatal(
        SourceRef const& location, std::string const& message
    );
    void error(std::string const& message);
    void error(SourceRef const& location, std::string const& message);
    void warning(std::string const& message);
    void warning(SourceRef const& location, std::string const& message);

    template<typename ReturnType>
    ReturnType error(std::string const& message, ReturnType&& returnValue)
    {
        error(message);
        return std::forward<ReturnType>(returnValue);
    }

    template<typename ReturnType>
    ReturnType error(
        SourceRef const& location,
        std::string const& message,
        ReturnType&& returnValue
    )
    {
        error(location, message);
        return std::forward<ReturnType>(returnValue);
    }

    template<typename ReturnType>
    ReturnType warning(std::string const& message, ReturnType&& returnValue)
    {
        warning(message);
        return std::forward<ReturnType>(returnValue);
    }

    template<typename ReturnType>
    ReturnType warning(
        SourceRef const& location,
        std::string const& message,
        ReturnType&& returnValue
    )
    {
        warning(location, message);
        return std::forward<ReturnType>(returnValue);
    }
};