//============================================================================
// @author      : Thomas Dooms & Ward Gauderis
// @date        : 3/10/20
// @copyright   : BA2 Informatica - Thomas Dooms & Ward Gauderis - University of Antwerp
//============================================================================

#pragma once

#include <stdexcept>
#include <memory>
#include <ostream>

class CompilationError : public std::exception {
public:
	static std::string file;

	explicit CompilationError(const std::string& message, const unsigned int line = 0, const unsigned int column = 0,
			bool warning = false)
			:CompilationError(message, line, column, warning, "error") { }

	[[nodiscard]] const char* what() const noexcept final
	{
		return message.c_str();
	}

	friend std::ostream& operator<<(std::ostream& os, const CompilationError& error)
	{
		os << error.message;
		return os;
	}

protected:

	explicit CompilationError(const std::string& message, const unsigned int line, const unsigned int column,
			bool warning, const std::string& type)
	{
		CompilationError::message = "\033[1m"+
				file+":"+
				(line ? std::to_string(line)+":" : "")+
				(column ? std::to_string(column)+":" : "")+(warning ? " \033[1;33m" : " \033[1;31m")+
				type+(warning ? " warning" : " error")+":\033[0m "+
				message+"\n";
	}

private:
	std::string message;
};

class SyntaxError : public CompilationError {
public:
	explicit SyntaxError(const std::string& message, const unsigned int line = 0, const unsigned int column = 0.,
			bool warning = false)
			:CompilationError(message, line, column, warning, "syntax") { }
};

class SemanticError : public CompilationError {
public:
	explicit SemanticError(const std::string& message, const unsigned int line = 0, const unsigned int column = 0.,
			bool warning = false)
			:CompilationError(message, line, column, warning, "semantic") { }
};

class UndeclaredError : public SemanticError {
public:
	explicit UndeclaredError(const std::string& symbol, unsigned int line = 0, unsigned int column = 0)
			:SemanticError("'"+symbol+"' undeclared", line, column) { }
};

class InternalError : public CompilationError {
public:
	explicit InternalError(const std::string& message, const unsigned int line = 0, const unsigned int column = 0.,
			bool warning = false)
			:CompilationError(message, line, column, warning, "internal") { }
};
