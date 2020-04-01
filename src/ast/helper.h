//============================================================================
// @author      : Thomas Dooms
// @date        : 3/30/20
// @copyright   : BA2 Informatica - Thomas Dooms - University of Antwerp
//============================================================================


#pragma once

#include "expressions.h"

struct Helper
{
    template <typename Type>
    static bool folder(Type*& elem)
    {
        if(not elem) return true;
        if(auto* folded = elem->fold())
        {
            if(auto* res = dynamic_cast<Type*>(folded))
            {
                elem = res;
            }
        }
        else return true;
        return false;
    }

    template <typename Type0, typename Type1>
    static Ast::Literal*
    fold_modulo(Type0 lhs, Type1 rhs, std::shared_ptr<SymbolTable> table, size_t line, size_t column)
    {
        if constexpr(std::is_integral_v<Type0> and std::is_integral_v<Type1>)
        {
            return new Ast::Literal(lhs % rhs, std::move(table), line, column);
        }
        else
        {
            throw InternalError("modulo on floating points while folding", line, column);
        }
    }

    template <typename Variant0, typename Variant1>
    static Ast::Literal*
    fold_binary(Variant0 lhs, Variant1 rhs, BinaryOperation operation, std::shared_ptr<SymbolTable> table, size_t line, size_t column)
    {
        if(operation.isDivisionModulo() and rhs == 0) return nullptr;

        if(operation == BinaryOperation::Add)
            return new Ast::Literal(lhs + rhs, std::move(table), line, column);
        else if(operation == BinaryOperation::Sub)
            return new Ast::Literal(lhs - rhs, std::move(table), line, column);
        else if(operation == BinaryOperation::Mul)
            return new Ast::Literal(lhs * rhs, std::move(table), line, column);
        else if(operation == BinaryOperation::Div)
            return new Ast::Literal(lhs / rhs, std::move(table), line, column);
        else if(operation == BinaryOperation::Mod)
            return fold_modulo(lhs, rhs, std::move(table), line, column);
        else if(operation == BinaryOperation::Lt)
            return new Ast::Literal(lhs < rhs, std::move(table), line, column);
        else if(operation == BinaryOperation::Gt)
            return new Ast::Literal(lhs > rhs, std::move(table), line, column);
        else if(operation == BinaryOperation::Le)
            return new Ast::Literal(lhs <= rhs, std::move(table), line, column);
        else if(operation == BinaryOperation::Ge)
            return new Ast::Literal(lhs >= rhs, std::move(table), line, column);
        else if(operation == BinaryOperation::Eq)
            return new Ast::Literal(lhs == rhs, std::move(table), line, column);
        else if(operation == BinaryOperation::Neq)
            return new Ast::Literal(lhs != rhs, std::move(table), line, column);
        else if(operation == BinaryOperation::And)
            return new Ast::Literal(lhs && rhs, std::move(table), line, column);
        else if(operation == BinaryOperation::Or)
            return new Ast::Literal(lhs || rhs, std::move(table), line, column);
        else
            throw InternalError("unknown binary operation", line, column);
    }

    template <typename Variant>
    static Ast::Literal*
    fold_prefix(Variant operand, PrefixOperation operation, std::shared_ptr<SymbolTable> table, size_t line, size_t column)
    {
        if(operation == PrefixOperation::Plus)
            return new Ast::Literal(operand, std::move(table), line, column);
        else if(operation == PrefixOperation::Neg)
            return new Ast::Literal(-operand, std::move(table), line, column);
        else if(operation == PrefixOperation::Not)
            return new Ast::Literal(!operand, std::move(table), line, column);
        else if(operation == PrefixOperation::Incr)
            return new Ast::Literal(operand + 1, std::move(table), line, column);
        else if(operation == PrefixOperation::Decr)
            return new Ast::Literal(operand - 1, std::move(table), line, column);
        else if(operation == PrefixOperation::Deref or operation == PrefixOperation::Addr)
            return nullptr;
        else
            throw InternalError("unknown prefix expression", line, column);
    }

    template <typename Variant>
    static Ast::Literal*
    fold_postfix(Variant operand, PostfixOperation operation, std::shared_ptr<SymbolTable> table, size_t line, size_t column)
    {
        if(operation == PostfixOperation::Incr)
            return new Ast::Literal(operand + 1, std::move(table), line, column);
        else if(operation == PostfixOperation::Decr)
            return new Ast::Literal(operand - 1, std::move(table), line, column);
        else
            throw InternalError("unknown postfix expression", line, column);
    }

    template <typename Variant>
    static Ast::Literal*
    fold_cast(Variant operand, const Type& type, std::shared_ptr<SymbolTable> table, size_t line, size_t column)
    {
        if(type.isFloatType())
            return new Ast::Literal((float)operand, std::move(table), line, column);
        else if(type.isCharacterType())
            return new Ast::Literal((char)operand, std::move(table), line, column);
        else if(type.isIntegerType() or type.isPointerType())
            return new Ast::Literal((int)operand, std::move(table), line, column);
        else
            throw InternalError("unknown type for conversion: " + type.string(), line, column);
    }

    static bool evaluate(Ast::Literal* literal)
    {
        const auto lambda = [](const auto& val) { return static_cast<bool>(val); };
        return std::visit(lambda, literal->literal);
    }

    static bool is_lvalue(Ast::Expr* expr)
    {
        if(dynamic_cast<Ast::Variable*>(expr))
        {
            return true;
        }
        else if(auto* res = dynamic_cast<Ast::PrefixExpr*>(expr))
        {
            return res->operation == PrefixOperation::Deref;
        }
        else if(auto* res = dynamic_cast<Ast::PostfixExpr*>(expr))
        {
            // TODO there are some stuffs here
        }
        return false;
    }
};
