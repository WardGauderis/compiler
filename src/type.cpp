//============================================================================
// @author      : Thomas Dooms
// @date        : 3/16/20
// @copyright   : BA2 Informatica - Thomas Dooms - University of Antwerp
//============================================================================

#include "type.h"

namespace
{
template<class... Ts>
struct overloaded : Ts...
{
    using Ts::operator()...;
};
template<class... Ts>
overloaded(Ts...)->overloaded<Ts...>;
} // namespace

[[nodiscard]] std::string Type::string() const
{
    return std::visit(
        overloaded {[&](const Type* ptr) {
                        return ptr->string() + "*" + (isTypeConst ? " const" : "");
                    },
                    [&](BaseType base) {
                        return (isTypeConst ? "const " : "") + toString(base);
                    }},
        type);
}

BaseType Type::getBaseType() const
{
    if (isBaseType()) return std::get<BaseType>(type);
}

bool Type::isConst() const
{
    return isTypeConst;
}

bool Type::isBaseType() const
{
    return type.index() == 1;
}

bool Type::isPointerType() const
{
    return type.index() == 0;
}

bool Type::isIntegralType() const
{
    return isBaseType()
           and (getBaseType() == BaseType::Char or getBaseType() == BaseType::Short or getBaseType() == BaseType::Int or getBaseType() == BaseType::Long);
}

bool Type::isCharacterType() const
{
    return isBaseType() and getBaseType() == BaseType::Char;
}

bool Type::isIntegerType() const
{
    return isBaseType() and getBaseType() == BaseType::Int;
}

bool Type::isFloatingType() const
{
    return isBaseType() and (getBaseType() == BaseType::Float or getBaseType() == BaseType::Double);
}

std::string Type::toString(BaseType type)
{
    switch (type)
    {
    case BaseType::Char:
        return "char";
    case BaseType::Short:
        return "short";
    case BaseType::Int:
        return "int";
    case BaseType::Long:
        return "long";
    case BaseType::Float:
        return "float";
    case BaseType::Double:
        return "double";
    default:
        throw InternalError("unknown base type");
    }
}

BaseType Type::fromString(const std::string& str)
{
    if (str == "char") return BaseType::Char;
    else if (str == "short")
        return BaseType::Short;
    else if (str == "int")
        return BaseType::Int;
    else if (str == "long")
        return BaseType::Long;
    else if (str == "float")
        return BaseType::Float;
    else if (str == "double")
        return BaseType::Double;
    else
        throw InternalError("string cannot convert to base type");
}

Type Type::unary(PrefixOperation operation, Type operand, size_t line, size_t column)
{
    if(operation == PrefixOperation::Not)
    {
        return Type(false, BaseType::Int);
    }
    else if(operand.isPointerType())
    {
        throw InvalidOperands("prefix operator", operand.string(), line, column);
    }
    return operand;
}

Type Type::combine(BinaryOperation operation, Type lhs, Type rhs, size_t line, size_t column)
{
    if (operation.isLogicalOperator())
    {
        return Type(false, BaseType::Int); // TODO: make this a bool
    }

    if (lhs.isBaseType() and rhs.isBaseType() and not lhs.isFloatingType() and not rhs.isFloatingType())
    {
        return Type(false, std::max(lhs.getBaseType(), rhs.getBaseType()));
    }
    else if (lhs.isPointerType() and rhs.isPointerType())
    {
        // empty statement, just go to throw
    }
    else if (lhs.isPointerType() and rhs.isIntegralType() and operation.isLogicalOperator())
    {
        return lhs;
    }
    else if (rhs.isPointerType() and lhs.isIntegralType() and operation.isLogicalOperator())
    {
        return rhs;
    }
    throw InvalidOperands(operation.string(), lhs.string(), rhs.string(), line, column);
}

std::optional<SemanticError> Type::convert(Type from, Type to, bool cast, size_t line, size_t column)
{
    std::string operation = cast ? "casting" : "assigning";
    if (from.isPointerType())
    {
        if (to.isFloatingType())
        {
            return ConversionError(operation, from.string(), to.string(), line, column);
        }
        else if (to.isIntegralType() and not cast)
        {
            return PointerConversionWarning(operation, "from", from.string(), to.string(), line, column);
        }
    }
    if (to.isPointerType())
    {
        if (from.isFloatingType())
        {
            return ConversionError(operation, from.string(), to.string(), line, column);
        }
        else if (from.isIntegralType() and not cast)
        {
            return PointerConversionWarning(operation, "to", from.string(), to.string(), line, column);
        }
    }
    return {};
}