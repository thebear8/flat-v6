#pragma once
#include <string>
#include <string_view>
#include <ostream>

struct ASTNode;

class ErrorLogger
{
private:
	std::string_view source;
	std::ostream& output;

public:
	ErrorLogger(std::string_view source, std::ostream& output) :
		source(source), output(output) { }

public:
	[[noreturn]] void error(std::string const& message);
	[[noreturn]] void error(ASTNode* node, std::string const& message);
	[[noreturn]] void error(size_t position, std::string const& message);
	[[noreturn]] void error(size_t begin, size_t end, std::string const& message);
	void warning(std::string const& message);
	void warning(ASTNode* node, std::string const& message);
	void warning(size_t position, std::string const& message);
	void warning(size_t begin, size_t end, std::string const& message);

	template<typename ReturnType>
	ReturnType error(ASTNode* node, std::string const& message, ReturnType&& returnValue)
	{
		error(node, message);
		return std::forward<ReturnType>(returnValue);
	}

	template<typename ReturnType>
	ReturnType warning(ASTNode* node, std::string const& message, ReturnType&& returnValue)
	{
		warning(node, message);
		return std::forward<ReturnType>(returnValue);
	}
};