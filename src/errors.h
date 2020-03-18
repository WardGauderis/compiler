//============================================================================
// @author      : Thomas Dooms & Ward Gauderis
// @date        : 3/10/20
// @copyright   : BA2 Informatica - Thomas Dooms & Ward Gauderis - University of Antwerp
//============================================================================

#pragma once

#include <antlr4-runtime/tree/ParseTree.h>
#include <memory>
#include <ostream>
#include <stdexcept>

class CompilationError : public std::exception
{
public:
    static std::string file;

    explicit CompilationError(
        const std::string& message, const unsigned int line = 0, const unsigned int column = 0, bool warning = false)
        : CompilationError(message, line, column, warning, "error")
    {
    }

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
    explicit CompilationError(
        const std::string& message, const unsigned int line, const unsigned int column, bool warning, const std::string& type)
    {
        CompilationError::message
            = "\033[1m" + file + ":" + (line ? std::to_string(line) + ":" : "")
              + (column ? std::to_string(column) + ":" : "") + (warning ? " \033[1;33m" : " \033[1;31m")
              + type + (warning ? " warning" : " error") + ":\033[0m " + message + "\n";
    }

private:
    std::string message;
};

struct SyntaxError : public CompilationError
{
    explicit SyntaxError(const std::string& message, const unsigned int line = 0, const unsigned int column = 0., bool warning = false)
        : CompilationError(message, line, column, warning, "syntax")
    {
    }
};

struct SemanticError : public CompilationError
{
    explicit SemanticError(const std::string& message, const unsigned int line = 0, const unsigned int column = 0., bool warning = false)
        : CompilationError(message, line, column, warning, "semantic")
    {
    }
};

struct UndeclaredError : public SemanticError
{
    explicit UndeclaredError(const std::string& symbol, unsigned int line = 0, unsigned int column = 0)
        : SemanticError("'" + symbol + "' not yet declared", line, column)
    {
    }
};

struct RedefinitionError : public SemanticError
{
    explicit RedefinitionError(const std::string& symbol, unsigned int line = 0, unsigned int column = 0)
        : SemanticError("'" + symbol + "' already defined", line, column)
    {
    }
};

struct ConstError : public SemanticError
{
    explicit ConstError(const std::string& operation, const std::string& name, unsigned int line = 0, unsigned int column = 0)
        : SemanticError(operation + " of const variable '" + name + "' is not allowed", line, column)
    {
    }
};

struct InternalError : public CompilationError
{
    explicit InternalError(const std::string& message, const unsigned int line = 0, const unsigned int column = 0., bool warning = false)
        : CompilationError(message, line, column, warning, "internal")
    {
    }
};

struct UnexpectedContextType : public InternalError
{
    explicit UnexpectedContextType(
        antlr4::tree::ParseTree* context, const unsigned int line = 0, const unsigned int column = 0., bool warning = false)
        : InternalError(std::string("unexpected context type: ") + typeid(*context).name(), line, column, warning)
    {
    }
};

struct LiteralOutOfRange : public SemanticError
{
    explicit LiteralOutOfRange(
        const std::string& literal, const unsigned int line = 0, const unsigned int column = 0., bool warning = true)
        : SemanticError("literal: " + literal + " out of range", line, column, warning)
    {
    }
};

struct InvalidOperands : public SemanticError
{
    explicit InvalidOperands(
        const std::string& operation,
        const std::string& lhs,
        const std::string& rhs,
        const unsigned int line   = 0,
        const unsigned int column = 0.)
        : SemanticError(
            "invalid operands to binary " + operation + "(has '" + lhs + " and " + rhs + "')", line, column, true)
    {
    }
    explicit InvalidOperands(
        const std::string& operation, const std::string& operand, const unsigned int line = 0, const unsigned int column = 0.)
        : SemanticError("invalid operands to unary " + operation + "(has '" + operand + "')", line, column, true)
    {
    }
};

struct ConversionError : public SemanticError
{
    explicit ConversionError(
        const std::string& operation, const std::string& from, const std::string& to, size_t line = 0, size_t column = 0)
        : SemanticError(operation + " to incompatible type (from '" + from + "' to '" + to + "')", line, column, false)
    {
    }
};

struct PointerConversionWarning : public SemanticError
{
    explicit PointerConversionWarning(
        const std::string& operation,
        const std::string& whence,
        const std::string& from,
        const std::string& to,
        size_t line   = 0,
        size_t column = 0)
        : SemanticError(
            operation + " " + whence + " pointer type without cast (from '" + from + "' to '" + to
                + "')",
            line,
            column,
            true)
    {
    }
};
