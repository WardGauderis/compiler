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
    return isBaseType() and (getBaseType() == BaseType::Char or
    getBaseType() == BaseType::Short or
    getBaseType() == BaseType::Int or
    getBaseType() == BaseType::Long);
}

bool Type::isCharacterType() const
{
	return isBaseType() and getBaseType() == BaseType::Char;
}

bool Type::isFloatingType() const
{
    return isBaseType() and (getBaseType() == BaseType::Float or
                             getBaseType() == BaseType::Double);
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
    else throw InternalError("string cannot convert to base type");
}

BaseType Type::combine(Type lhs, Type rhs)
{
    if (lhs.isBaseType() and rhs.isBaseType())
    {
        return std::max(lhs.getBaseType(), rhs.getBaseType());
    }
    else
    {
        throw InternalError("pointer cast stuff not yet defined");
    }
}