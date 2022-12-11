#include "error_logger.hpp"

#include "../data/ast.hpp"

void ErrorLogger::error(std::string const& message)
{
	output << "Error: " << message << "\n";
	throw std::exception(message.c_str());
}

void ErrorLogger::error(ASTNode* node, std::string const& message)
{
	return error(node->begin, node->end, message);
}

void ErrorLogger::error(size_t position, std::string const& message)
{
	size_t line = 1, column = 1;
	for (size_t i = 0; i < position && i < source.length(); i++) {
		if (source[i] == '\n') {
			line++;
			column = 1;
		}
		else {
			column++;
		}
	}

	output << "Error: ln " << line << ", col " << column << ": " << message << "\n";
	throw std::exception(message.c_str());
}

void ErrorLogger::error(size_t begin, size_t end, std::string const& message)
{
	size_t line = 1, column = 1;
	for (size_t i = 0; i < begin && i < source.length(); i++) {
		if (source[i] == '\n') {
			line++;
			column = 1;
		}
		else {
			column++;
		}
	}

	output << "Error: ln " << line << ", col " << column << ", \"" << source.substr(begin, end - begin) << "\": " << message << "\n";
	throw std::exception(message.c_str());
}

void ErrorLogger::warning(std::string const& message)
{
	output << "Warning: " << message << "\n";
	throw std::exception(message.c_str());
}

void ErrorLogger::warning(ASTNode* node, std::string const& message)
{
	return warning(node->begin, node->end, message);
}

void ErrorLogger::warning(size_t position, std::string const& message)
{
	size_t line = 1, column = 1;
	for (size_t i = 0; i < position && i < source.length(); i++) {
		if (source[i] == '\n') {
			line++;
			column = 1;
		}
		else {
			column++;
		}
	}

	output << "Warning: ln " << line << ", col " << column << ": " << message << "\n";
}

void ErrorLogger::warning(size_t begin, size_t end, std::string const& message)
{
	size_t line = 1, column = 1;
	for (size_t i = 0; i < begin && i < source.length(); i++) {
		if (source[i] == '\n') {
			line++;
			column = 1;
		}
		else {
			column++;
		}
	}

	output << "Warning: ln " << line << ", col " << column << ", \"" << source.substr(begin, end - begin) << "\": " << message << "\n";
}