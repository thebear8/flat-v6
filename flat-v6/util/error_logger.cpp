#include "error_logger.hpp"

void ErrorLogger::error(size_t position, std::string message)
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

void ErrorLogger::error(size_t begin, size_t end, std::string message)
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
#ifdef _DEBUG
	throw std::exception(message.c_str());
#endif
}

void ErrorLogger::warning(size_t position, std::string message)
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
	throw std::exception(message.c_str());
}

void ErrorLogger::warning(size_t begin, size_t end, std::string message)
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