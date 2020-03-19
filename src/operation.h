//============================================================================
// @author      : Thomas Dooms
// @date        : 3/18/20
// @copyright   : BA2 Informatica - Thomas Dooms - University of Antwerp
//============================================================================

#pragma once
#include <string>

struct BinaryOperation
{
    enum Type
    {
        Add,
        Sub,
        Mul,
        Div,
        Mod,
        Lt,
        Gt,
        Le,
        Ge,
        Eq,
        Neq,
        And,
        Or
    } type;

    BinaryOperation(Type operation) : type(operation)
    {
    }
    BinaryOperation(const std::string& str) : type(fromString(str).type)
    {
    }

    operator Type()
    {
        return type;
    }
    static BinaryOperation fromString(const std::string& str);

    std::string string() const;

    bool isLogicalOperator() const;
    bool isComparisonOperator() const;
    bool isAdditiveOperator() const;
    bool isDivisionModulo() const;
};

struct PrefixOperation
{
    enum Type
    {
        Plus,
        Neg,
        Not,
        Incr,
        Decr,
    } type;

    PrefixOperation(Type operation) : type(operation)
    {
    }
    PrefixOperation(const std::string& str) : type(fromString(str).type)
    {
    }

    operator Type()
    {
        return type;
    }
    static PrefixOperation fromString(const std::string& str);

    std::string string() const;
};

struct PostfixOperation
{
    enum Type
    {
        Incr,
        Decr,
    } type;

    PostfixOperation(Type operation) : type(operation)
    {
    }
    PostfixOperation(const std::string& str) : type(fromString(str).type)
    {
    }

    operator Type()
    {
        return type;
    }
    PostfixOperation fromString(const std::string& str);

    std::string string() const;
};
