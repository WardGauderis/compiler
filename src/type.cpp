//============================================================================
// @author      : Thomas Dooms
// @date        : 3/16/20
// @copyright   : BA2 Informatica - Thomas Dooms - University of Antwerp
//============================================================================

#include "type.h"

namespace
{
template <class... Ts>
struct overloaded : Ts...
{
    using Ts::operator()...;
};
template <class... Ts>
overloaded(Ts...)->overloaded<Ts...>;
} // namespace

[[nodiscard]] std::string Type::string() const
{
    return std::visit(
    overloaded{ [&](std::monostate empty) { return std::string("void"); },
                [&](const Type* ptr) { return ptr->string() + "*" + (isTypeConst ? " const" : ""); },
                [&](BaseType base) { return (isTypeConst ? "const " : "") + toString(base); },
                [&](const FunctionType& func) {
                    auto res = func.returnType->string() + '(';
                    for(const auto& elem : func.parameters) res += elem->string() + ',';
                    if(func.variadic) res += "...";

                    if(res.back() == ',') res.back() = ')';
                    else
                        res += ')';

                    return res;
                },
                [&](const ArrayType& arr) {
                    return arr.second->string() + '[' + (arr.first ? std::to_string(arr.first) : "") + ']';
                } },
    type);
}

BaseType Type::getBaseType() const
{
    try
    {
        return std::get<BaseType>(type);
    }
    catch(...)
    {
        throw InternalError("wrong index when getting base type");
    }
}

const ArrayType& Type::getArrayType() const
{
    try
    {
        return std::get<ArrayType>(type);
    }
    catch(...)
    {
        throw InternalError("wrong index when getting array type");
    }
}

const FunctionType& Type::getFunctionType() const
{
    try
    {
        return std::get<FunctionType>(type);
    }
    catch(...)
    {
        throw InternalError("wrong index when getting function type");
    }
}

Type* Type::getDerefType() const
{
    if(isPointerType()) return std::get<Type*>(type);
    else if(isArrayType())
        return std::get<ArrayType>(type).second;
    else
        throw InternalError("type is not of form pointer/array type");
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

bool Type::isPointerLikeType() const
{
    return isPointerType() or isArrayType();
}

bool Type::isCharacterType() const
{
    return isBaseType() and getBaseType() == BaseType::Char;
}

bool Type::isIntegerType() const
{
    return isBaseType() and getBaseType() == BaseType::Int;
}

bool Type::isIntegralType() const
{
    return isCharacterType() or isIntegerType();
}

bool Type::isFloatType() const
{
    return isBaseType() and getBaseType() == BaseType::Float;
}

bool Type::isVoidType() const
{
    return type.index() == 0;
}

bool Type::isFunctionType() const
{
    return type.index() == 3;
}

bool Type::isArrayType() const
{
    return type.index() == 4;
}

bool operator==(const Type& lhs, const Type& rhs)
{
    if(lhs.type.index() != rhs.type.index()) return false;
    else if(lhs.isBaseType())
        return lhs.getBaseType() == rhs.getBaseType();
    else if(lhs.isVoidType())
    {
        return rhs.isVoidType();
    }
    else
        return (*lhs.getDerefType()) == (*rhs.getDerefType());
}

bool operator!=(const Type& lhs, const Type& rhs)
{
    return not(lhs == rhs);
}

std::string Type::toString(BaseType type)
{
    switch(type)
    {
    case BaseType::Char:
        return "char";
    case BaseType::Int:
        return "int";
    case BaseType::Float:
        return "float";
    default:
        throw InternalError("unknown base type");
    }
}

Type* Type::decay(Type* type)
{
    std::vector<bool> consts;
    while(type->isArrayType())
    {
        consts.emplace_back(type->isTypeConst);
        type = type->getDerefType();
    }
    for(const auto& i : consts)
    {
        type = new Type(i, type);
    }
    return type;
}

Type* Type::invert(Type* type)
{
    std::vector<size_t> vec;
    while(type->isArrayType())
    {
        vec.emplace_back(type->getArrayType().first);
        type = type->getArrayType().second;
    }
    for(const auto& elem : vec)
    {
        type = new Type(false, elem, type);
    }
    return type;
}

Type* Type::unary(PrefixOperation operation, Type* operand, size_t line, size_t column, bool print)
{
    if(operation == PrefixOperation::Deref)
    {
        if(operand->isPointerType())
        {
            return operand->getDerefType();
        }
        else if(operand->isArrayType())
        {
            return operand->getArrayType().second;
        }
        else
        {
            std::cout << SemanticError("cannot dereference non-pointer type " + operand->string(), line, column);
            return nullptr;
        }
    }
    if(operation == PrefixOperation::Addr)
    {
        return new Type(false, operand);
    }

    if(operation == PrefixOperation::Not)
    {
        return new Type(false, BaseType::Int);
    }
    else if((operation == PrefixOperation::Plus or operation == PrefixOperation::Neg) and operand->isPointerType())
    {
        if(print) std::cout << InvalidOperands(operation.string(), operand->string(), line, column);
        return nullptr;
    }
    return operand;
}

Type* Type::combine(BinaryOperation operation, Type* lhs, Type* rhs, size_t line, size_t column, bool print)
{
    if(operation.isLogicalOperator())
    {
        return new Type(false, BaseType::Int); // TODO: make this a bool
    }

    if(operation.isComparisonOperator()
       and ((rhs->isPointerType() and lhs->isFloatType()) or (rhs->isFloatType() and lhs->isPointerType())))
    {
        // empty statement, just go to throw
    }
    else if(lhs->isBaseType() and rhs->isBaseType())
    {
        if(operation == BinaryOperation::Mod and (lhs->isFloatType() or rhs->isFloatType()))
        {
        }
        else if(operation.isComparisonOperator())
        {
            return new Type(false, BaseType::Int); // TODO: make this a bool
        }
        else
            return new Type(false, std::max(lhs->getBaseType(), rhs->getBaseType()));
    }
    else if(lhs->isPointerLikeType() and rhs->isPointerLikeType())
    {
        if(operation.isComparisonOperator())
        {
            return new Type(false, BaseType::Int); // must be bool
        }
    }
    else if(lhs->isPointerLikeType() and rhs->isIntegralType() and operation.isAdditiveOperator())
    {
        return lhs;
    }
    else if(lhs->isIntegralType() and rhs->isPointerLikeType() and operation == BinaryOperation::Add)
    {
        return rhs;
    }
    if(print)
        std::cout << InvalidOperands(operation.string(), lhs->string(), rhs->string(), line, column);
    return nullptr;
}

bool Type::convert(Type* from, Type* to, bool cast, size_t line, size_t column, bool print)
{
    std::string operation = cast ? "casting" : "assigning";

    // cannot convert to array type
    if(to->isArrayType())
    {
        if(print)
            std::cout << ConversionError(operation, from->string(), to->string(), line, column);
        return false;
    }

    // cannot convert from or to void
    if((from->isVoidType() and not to->isVoidType()) or (not from->isVoidType() and to->isVoidType()))
    {
        if(print)
            std::cout << ConversionError(operation, from->string(), to->string(), line, column);
        return false;
    }

    // cannot convert float to ptr
    if(from->isPointerLikeType())
    {
        if(to->isFloatType())
        {
            if(print)
                std::cout << ConversionError(operation, from->string(), to->string(), line, column);
            return false;
        }
        else if(to->isIntegralType() and not cast)
        {
            if(print)
                std::cout
                << PointerConversionWarning(operation, "from", from->string(), to->string(), line, column);
        }
        else if(from->isArrayType() and not to->isPointerType())
        {
            if(print)
                std::cout << ConversionError(operation, from->string(), to->string(), line, column);
            return false;
        }
    }

    // cannot convert float to ptr
    if(to->isPointerLikeType())
    {
        if(from->isFloatType())
        {
            if(print)
                std::cout << ConversionError(operation, from->string(), to->string(), line, column);
            return false;
        }
        else if(from->isIntegralType() and not cast)
        {
            if(print)
                std::cout
                << PointerConversionWarning(operation, "to", from->string(), to->string(), line, column);
        }
    }

    // casting to narrower type
    if(from->isBaseType() and to->isBaseType() and to->getBaseType() < from->getBaseType() and not cast)
    {
        std::cout << NarrowingConversion(operation, from->string(), to->string(), line, column);
    }
    // casting to narrower basetype
    if(not cast and from->isPointerType() and to->isPointerType() and (*from) != (*to))
    {
        if(print)
            std::cout << PointerConversionWarning(operation, "to", from->string(), to->string(), line, column);
    }

    // converting ptr to char is very narrowing
    if(from->isPointerType() and to->isCharacterType())
    {
        std::cout << NarrowingConversion(operation, from->string(), to->string(), line, column);
    }

    return true;
}