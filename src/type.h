//============================================================================
// @author      : Thomas Dooms
// @date        : 3/16/20
// @copyright   : BA2 Informatica - Thomas Dooms - University of Antwerp
//============================================================================

#pragma once

#include "errors.h"
#include <memory>
#include <string>
#include <variant>

namespace
{
template<typename Variant, typename Type, std::size_t index = 0>
constexpr std::size_t variant_index()
{
    if constexpr (index == std::variant_size_v<Variant>)
    {
        return index;
    }
    else if constexpr (std::is_same_v<std::variant_alternative_t<index, Variant>, Type>)
    {
        return index;
    }
    else
    {
        return variant_index<Variant, Type, index + 1>();
    }
}
} // namespace
using TypeVariant = std::variant<char, short, int, long, float, double>;

enum class BaseType
{
    Char   = variant_index<TypeVariant, char>(),
    Short  = variant_index<TypeVariant, short>(),
    Int    = variant_index<TypeVariant, int>(),
    Long   = variant_index<TypeVariant, long>(),
    Float  = variant_index<TypeVariant, float>(),
    Double = variant_index<TypeVariant, double>(),
};

class Type
{
public:
    explicit Type(bool isConst, Type* ptr) : isTypeConst(isConst), type(ptr)
    {
    }

    explicit Type(bool isConst, const std::string& baseType)
        : isTypeConst(isConst), type(fromString(baseType))
    {
    }

    explicit Type(bool isConst, BaseType baseType) : isTypeConst(isConst), type(baseType)
    {
    }

    [[nodiscard]] std::string string() const;

    [[nodiscard]] BaseType getBaseType() const;

    [[nodiscard]] bool isConst() const;

    [[nodiscard]] bool isBaseType() const;

    [[nodiscard]] bool isPointerType() const;

    [[nodiscard]] bool isIntegralType() const;

	[[nodiscard]] bool isCharacterType() const;

	[[nodiscard]] bool isFloatingType() const;

    static std::string toString(BaseType type);
    static BaseType fromString(const std::string& str);
    static BaseType combine(Type lhs, Type rhs);

private:
    bool isTypeConst;
    // do not change the order of this variant
    std::variant<Type*, BaseType> type;
};