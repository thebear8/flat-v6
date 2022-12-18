#include "error_logger.hpp"

#include <sstream>

#include "../data/ast.hpp"

void ErrorLogger::fatal(std::string const& message)
{
    std::stringstream text;
    text << "Fatal error: " << message << "\n";
    output << text.str();
    throw std::exception(text.str().c_str());
}

void ErrorLogger::fatal(SourceRef const& location, std::string const& message)
{
    if (!sources.contains(location.id))
        fatal(message);

    auto const& source = sources.at(location.id);
    size_t line = 1, column = 1;
    for (size_t i = 0; i < location.begin && i < source.length(); i++)
    {
        line = ((source[i] == '\n') ? ++line : line);
        column = ((source[i] == '\n') ? 1 : ++column);
    }

    std::stringstream text;
    text << "Fatal error: ln " << line << ", col " << column;
    if (location.begin != location.end)
        text << source.substr(
            std::min(location.begin, source.size() - 1),
            location.end - location.begin);
    text << ": " << message << "\n";

    output << text.str();
    throw std::exception(text.str().c_str());
}

void ErrorLogger::error(std::string const& message)
{
    std::stringstream text;
    text << "Error: " << message << "\n";
    output << text.str();
#ifdef _DEBUG
    throw std::exception(text.str().c_str());
#endif  // NDEBUG
}

void ErrorLogger::error(SourceRef const& location, std::string const& message)
{
    if (!sources.contains(location.id))
        return error(message);

    auto const& source = sources.at(location.id);
    size_t line = 1, column = 1;
    for (size_t i = 0; i < location.begin && i < source.length(); i++)
    {
        line = ((source[i] == '\n') ? ++line : line);
        column = ((source[i] == '\n') ? 1 : ++column);
    }

    std::stringstream text;
    text << "Error: ln " << line << ", col " << column;
    if (location.begin != location.end)
        text << source.substr(
            std::min(location.begin, source.size() - 1),
            location.end - location.begin);
    text << ": " << message << "\n";

    output << text.str();
#ifdef _DEBUG
    throw std::exception(text.str().c_str());
#endif  // NDEBUG
}

void ErrorLogger::warning(std::string const& message)
{
    output << "Warning: " << message << "\n";
}

void ErrorLogger::warning(SourceRef const& location, std::string const& message)
{
    if (!sources.contains(location.id))
        return warning(message);

    auto const& source = sources.at(location.id);
    size_t line = 1, column = 1;
    for (size_t i = 0; i < location.begin && i < source.length(); i++)
    {
        line = ((source[i] == '\n') ? ++line : line);
        column = ((source[i] == '\n') ? 1 : ++column);
    }

    std::stringstream text;
    text << "Warning: ln " << line << ", col " << column;
    if (location.begin != location.end)
        text << source.substr(
            std::min(location.begin, source.size() - 1),
            location.end - location.begin);
    text << ": " << message << "\n";

    output << text.str();
}