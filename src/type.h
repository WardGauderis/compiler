//============================================================================
// @author      : Thomas Dooms
// @date        : 3/16/20
// @copyright   : BA2 Informatica - Thomas Dooms - University of Antwerp
//============================================================================

#pragma once

#include "errors.h"
#include "operation.h"
#include <llvm/IR/Type.h>
#include <memory>
#include <string>
#include <variant>

namespace
{
template <typename Variant, typename Type, std::size_t index = 0>
constexpr std::size_t variant_index()
{
    if constexpr(index == std::variant_size_v<Variant>)
    {
        return index;
    }
    else if constexpr(std::is_same_v<std::variant_alternative_t<index, Variant>, Type>)
    {
        return index;
    }
    else
    {
        return variant_index<Variant, Type, index + 1>();
    }
}
} // namespace
using TypeVariant = std::variant<char, int, float>;

enum class BaseType
{
    Char  = variant_index<TypeVariant, char>(),
    Int   = variant_index<TypeVariant, int>(),
    Float = variant_index<TypeVariant, float>(),
};


class Type; // stupid predeclaration but oh well
struct FunctionType
{
    Type*              returnType;
    std::vector<Type*> parameters;
    bool               variadic;
};
using ArrayType = std::pair<size_t, Type*>;

class Type
{
    public: // default init to void
    explicit Type() : isTypeConst(false), type() {}
    explicit Type(bool isConst, Type* ptr) : isTypeConst(isConst), type(ptr) {}

    explicit Type(bool isConst, const std::string& str) : isTypeConst(isConst)
    {
        if(str == "char") type = BaseType::Char;
        else if(str == "int")
            type = BaseType::Int;
        else if(str == "float")
            type = BaseType::Float;
        else if(str == "void")
            type = std::monostate();
        else
            throw std::runtime_error("cannot convert string to type");
    }

    explicit Type(Type* ret, std::vector<Type*> params, bool variadic = false)
    : isTypeConst(true), type(FunctionType{ ret, std::move(params), variadic })
    {
    }

    explicit Type([[maybe_unused]] bool isConst, size_t size, Type* base)
    : isTypeConst(base->isConst()), type(std::make_pair(size, base))
    {
    }

    explicit Type(bool isConst, BaseType baseType) : isTypeConst(isConst), type(baseType) {}

    [[nodiscard]] std::string string(const std::string& name = "") const;

    [[nodiscard]] BaseType getBaseType() const;

    [[nodiscard]] const ArrayType& getArrayType() const;

    [[nodiscard]] const FunctionType& getFunctionType() const;

    [[nodiscard]] Type* getDerefType() const;

    [[nodiscard]] bool isConst() const;

    [[nodiscard]] bool isBaseType() const;

    [[nodiscard]] bool isPointerType() const;

    [[nodiscard]] bool isPointerLikeType() const;

    [[nodiscard]] bool isCharacterType() const;

    [[nodiscard]] bool isIntegralType() const;

    [[nodiscard]] bool isIntegerType() const;

    [[nodiscard]] bool isFloatType() const;

    [[nodiscard]] bool isVoidType() const;

    [[nodiscard]] bool isFunctionType() const;

    [[nodiscard]] bool isArrayType() const;

    friend bool operator==(const Type& lhs, const Type& rhs);

    friend bool operator!=(const Type& lhs, const Type& rhs);

    static std::string toString(BaseType type);

    [[nodiscard]] static Type* decay(Type* type);

    [[nodiscard]] static Type* invert(Type* type);

    [[nodiscard]] static Type*
    unary(PrefixOperation operation, Type* operand, size_t line = 0, size_t column = 0, bool print = true);

    [[nodiscard]] static Type*
    combine(BinaryOperation operation, Type* lhs, Type* rhs, size_t line = 0, size_t column = 0, bool print = true);

    [[nodiscard]] static bool convert(Type* from, Type* to, bool cast, size_t line = 0, size_t column = 0, bool print = true);

    private:
    bool isTypeConst;
    // do not change the order of this variant
    std::variant<std::monostate, Type*, BaseType, FunctionType, ArrayType> type;
};