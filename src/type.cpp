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
        overloaded {
                    [&](std::monostate empty) {
                        return std::string("undefined");
                    },
                    [&](const Type* ptr) {
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

std::optional<Type> Type::getDerefType() const
{
    if (isPointerType()) return *std::get<Type*>(type);
    else return std::nullopt;
}

bool Type::isConst() const
{
    return isTypeConst;
}

bool Type::isBaseType() const
{
    return type.index() == 2;
}

bool Type::isPointerType() const
{
    return type.index() == 1;
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

bool operator==(Type lhs, Type rhs)
{
    if (lhs.type.index() != rhs.type.index()) return false;
    else if (lhs.isBaseType()) return lhs.getBaseType() == rhs.getBaseType();
    else return *lhs.getDerefType() == *rhs.getDerefType();
}

bool operator!=(Type lhs, Type rhs)
{
    return not(lhs == rhs);
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

std::optional<Type> Type::unary(PrefixOperation operation, Type operand, size_t line, size_t column, bool print)
{
    if(operation == PrefixOperation::Not)
    {
        return Type(false, BaseType::Int);
    }
    else if((operation == PrefixOperation::Plus or operation == PrefixOperation::Neg) and operand.isPointerType())
    {
        if(print) std::cout << InvalidOperands(operation.string(), operand.string(), line, column);
        return std::nullopt;
    }
    return operand;
}

std::optional<Type> Type::combine(BinaryOperation operation, Type lhs, Type rhs, size_t line, size_t column, bool print)
{
    if (operation.isLogicalOperator())
    {
        return Type(false, BaseType::Int); // TODO: make this a bool
    }

    if (operation.isComparisonOperator() and ((rhs.isPointerType() and lhs.isFloatingType()) or (rhs.isFloatingType() and lhs.isPointerType())))
    {
        // empty statement, just go to throw
    }
    else if (lhs.isBaseType() and rhs.isBaseType())
    {
        if(operation == BinaryOperation::Mod and (lhs.isFloatingType() or rhs.isFloatingType()))
        {
        }
        else if(operation.isComparisonOperator())
        {
          return Type(false, BaseType::Int); // TODO: make this a bool
        }
        else return Type(false, std::max(lhs.getBaseType(), rhs.getBaseType()));
    }
    else if (lhs.isPointerType() and rhs.isPointerType())
    {
        if(operation.isComparisonOperator())
        {
            return Type(false, BaseType::Int); // must be bool
        }
    }
    else if (lhs.isPointerType() and rhs.isIntegralType() and operation.isAdditiveOperator())
    {
        return lhs;
    }
    else if (lhs.isIntegralType() and rhs.isPointerType() and operation == BinaryOperation::Add)
    {
        return rhs;
    }
    if(print) std::cout << InvalidOperands(operation.string(), lhs.string(), rhs.string(), line, column);
    return std::nullopt;
}

bool Type::convert(Type from, Type to, bool cast, size_t line, size_t column, bool print)
{
    std::string operation = cast ? "casting" : "assigning";

    if (from.isPointerType())
    {
        if (to.isFloatingType())
        {
            if(print) std::cout << ConversionError(operation, from.string(), to.string(), line, column);
            return false;
        }
        else if (to.isIntegralType() and not cast)
        {
            if(print) std::cout << PointerConversionWarning(operation, "from", from.string(), to.string(), line, column);
        }
    }
    if (to.isPointerType())
    {
        if (from.isFloatingType())
        {
            if(print) std::cout << ConversionError(operation, from.string(), to.string(), line, column);
            return false;
        }
        else if (from.isIntegralType() and not cast)
        {
            if(print) std::cout << PointerConversionWarning(operation, "to", from.string(), to.string(), line, column);
        }
    }
    if(from.isBaseType() and to.isBaseType() and to.getBaseType() < from.getBaseType() and not cast)
    {
        std::cout << NarrowingConversion(operation, from.string(), to.string(), line, column);
    }
    if(not cast and from.isPointerType() and to.isPointerType() and from != to)
    {
        std::cout << PointerConversionWarning(operation, "to", from.string(), to.string(), line, column);
    }
    if(from.isPointerType() and to.isBaseType() and to.getBaseType() != BaseType::Long)
    {
        std::cout << NarrowingConversion(operation, from.string(), to.string(), line, column);
    }

    return true;
}