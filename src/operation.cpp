//============================================================================
// @author      : Thomas Dooms
// @date        : 3/18/20
// @copyright   : BA2 Informatica - Thomas Dooms - University of Antwerp
//============================================================================

#include "operation.h"
#include "errors.h"

BinaryOperation BinaryOperation::fromString(const std::string& str)
{
    if (str == "+") return BinaryOperation::Add;
    else if (str == "-") return BinaryOperation::Sub;
    else if (str == "*") return BinaryOperation::Mul;
    else if (str == "/") return BinaryOperation::Div;
    else if (str == "%") return BinaryOperation::Mod;
    else if (str == "<") return BinaryOperation::Lt;
    else if (str == ">") return BinaryOperation::Gt;
    else if (str == "<=") return BinaryOperation::Le;
    else if (str == ">=") return BinaryOperation::Ge;
    else if (str == "==") return BinaryOperation::Eq;
    else if (str == "!=") return BinaryOperation::Neq;
    else if (str == "&&") return BinaryOperation::And;
    else if (str == "||") return BinaryOperation::Or;
    else throw InternalError("unknown binary operation");
}

std::string BinaryOperation::string() const
{
    if (type == BinaryOperation::Add) return "+";
    else if (type == BinaryOperation::Sub) return "-";
    else if (type == BinaryOperation::Mul) return "*";
    else if (type == BinaryOperation::Div) return "/";
    else if (type == BinaryOperation::Mod) return "%";
    else if (type == BinaryOperation::Lt) return "<";
    else if (type == BinaryOperation::Gt) return ">";
    else if (type == BinaryOperation::Le) return "<=";
    else if (type == BinaryOperation::Ge) return ">=";
    else if (type == BinaryOperation::Eq) return "==";
    else if (type == BinaryOperation::Neq) return "!=";
    else if (type == BinaryOperation::And) return "&&";
    else if (type == BinaryOperation::Or) return "||";
    else throw InternalError("unknown binary operation");
}

bool BinaryOperation::isLogicalOperator() const
{
    return type == And or type == Or;
}

bool BinaryOperation::isAdditiveOperator() const
{
    return type == Add or type == Sub;
}

bool BinaryOperation::isDivisionModulo() const
{
    return type == Div or type == Mod;
}

PrefixOperation PrefixOperation::fromString(const std::string& str)
{
    if (str == "+") return PrefixOperation::Plus;
    else if (str == "-") return PrefixOperation::Neg;
    else if (str == "!") return PrefixOperation::Not;
    else if(str == "++") return PrefixOperation::Incr;
    else if(str == "--") return PrefixOperation::Decr;
    else throw InternalError("unknown prefix operation string: " + str);
}

std::string PrefixOperation::string() const
{
    if (type == PrefixOperation::Plus) return "+";
    else if (type == PrefixOperation::Neg) return "-";
    else if (type == PrefixOperation::Not) return "!";
    else if (type == PrefixOperation::Incr) return "++";
    else if (type == PrefixOperation::Decr) return "--";
    else throw InternalError("unknown prefix operation");
}

PostfixOperation PostfixOperation::fromString(const std::string& str)
{
    if(str == "++") return PostfixOperation::Incr;
    else if(str == "--") return PostfixOperation::Decr;
    else throw InternalError("unknown postfix operation string: " + str);
}

std::string PostfixOperation::string() const
{
    if (type == PostfixOperation::Incr) return "++";
    else if (type == PostfixOperation::Decr) return "--";
    else throw InternalError("unknown postfix operation");
}
