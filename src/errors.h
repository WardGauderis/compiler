//============================================================================
// @author      : Thomas Dooms & Ward Gauderis
// @date        : 3/10/20
// @copyright   : BA2 Informatica - Thomas Dooms & Ward Gauderis - University of Antwerp
//============================================================================

#pragma once

#include <stdexcept>
#include <memory>

class CompilationError : public std::exception {
public:
	static std::string file;

	explicit CompilationError(const std::string& error, const unsigned int line = 0, const unsigned int column = 0)
	{
		message = "\033[1m"+
				file+":"+
				(line ? std::to_string(line)+":" : "")+
				(column ? std::to_string(column)+":" : "")+" \033[1;31m"+
				type()+":\033[0m "+
				error;

	}

	[[nodiscard]] const char* what() const noexcept final
	{
		return message.c_str();
	}

private:
	std::string message;

	[[nodiscard]] virtual std::string type() const
	{
		return "compilation error";
	}
};

class SyntaxError : public CompilationError {
public:
	using CompilationError::CompilationError;
private:
	[[nodiscard]] std::string type() const final
	{
		return "syntax error";
	}
};

class SemanticError : public CompilationError {
public:
	using CompilationError::CompilationError;
private:
	[[nodiscard]] std::string type() const final
	{
		return "semantic error";
	}
};

class UndeclaredError : public SemanticError {
public:
	explicit UndeclaredError(const std::string& symbol, unsigned int line = 0, unsigned int column = 0)
			:SemanticError("'"+symbol+"' undeclared", line, column) { }
};

class InternalError : public CompilationError {
public:
	using CompilationError::CompilationError;
private:
	[[nodiscard]] std::string type() const final
	{
		return "internal error";
	}
};
