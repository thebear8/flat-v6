#pragma once
#include <string>
#include <string_view>
#include <ostream>

class ErrorLogger
{
private:
	std::string_view source;
	std::ostream& output;

public:
	ErrorLogger(std::string_view source, std::ostream& output) :
		source(source), output(output) { }

protected:
	void error(size_t position, std::string message);
	void error(size_t begin, size_t end, std::string message);
	void warning(size_t position, std::string message);
	void warning(size_t begin, size_t end, std::string message);
};