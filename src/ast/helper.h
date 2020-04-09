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
        else
        {
          return true;
        }
        return false;
    }

    template<typename Type, typename Func>
    static void remove_dead(std::vector<Type*>& children, Func&& pred)
    {
        for(size_t i = 0; i < children.size(); i++)
        {
            if(pred(children[i]))
            {
                children.resize(i + 1);
                return;
            }
        }
    }

    template<typename Type>
    static void fold_children(std::vector<Type*>& children)
    {
      for(auto iter = children.begin(); iter != children.end();)
      {
        if(Helper::folder(*iter))
        {
          iter = children.erase(iter);
        }
        else
        {
          iter++;
        }
      }
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
    fold_cast(Variant operand, Type* type, std::shared_ptr<SymbolTable> table, size_t line, size_t column)
    {
        if(type->isFloatType())
            return new Ast::Literal((float)operand, std::move(table), line, column);
        else if(type->isCharacterType())
            return new Ast::Literal((char)operand, std::move(table), line, column);
        else if(type->isIntegerType() or type->isPointerType())
            return new Ast::Literal((int)operand, std::move(table), line, column);
        else
            throw InternalError("unknown type for conversion: " + type->string(), line, column);
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
        else if(auto* res = dynamic_cast<Ast::SubscriptExpr*>(expr))
        {
            return true;
        }
        return false;
    }

    static bool fill_table_with_function(const std::vector<std::pair<Type*, std::string>>& parameters,
                                         Type*                               returnType,
                                         const std::string&                  identifier,
                                         const std::shared_ptr<SymbolTable>& table,
                                         const std::shared_ptr<SymbolTable>& scope,
                                         size_t                              line,
                                         size_t                              column)
    {

      // converting the parameter types
      std::vector<Type*> types(parameters.size());
      const auto         convert = [&](const auto& param) { return Type::decay(param.first); };
      std::transform(parameters.begin(), parameters.end(), types.begin(), convert);

      // checking for alternate redefinitions
      if(not table->insert(identifier, new Type(returnType, types), true))
      {
        auto* res = table->lookup(identifier);
        if(not res->type->isFunctionType())
        {
          std::cout << RedefinitionError(identifier, line, column);
          return false;
        }
        else if((*res->type->getFunctionType().returnType) != (*returnType))
        {
          std::cout << SemanticError("redefining function with different return type is not allowed", line, column);
          return false;
        }
        else if(res->type->getFunctionType().variadic)
        {
          std::cout << SemanticError("defining similar function without variadic elements is not allowed", line, column);
          return false;
        }
        else
        {
          if(res->type->getFunctionType().parameters.size() != types.size())
          {
            std::cout << SemanticError("overloading functions is not supported", line, column);
            return false;
          }

          for(size_t i = 0; i < types.size(); i++)
          {
            if((*res->type->getFunctionType().parameters[i]) != (*types[i]))
            {
              std::cout << SemanticError("overloading functions is not supported", line, column);
              return false;
            }
          }
        }
      }

        // checking for duplicate names by inserting into bogus scope
        for(const auto& [type, id] : parameters)
        {
            if(type->isVoidType())
            {
                std::cout << SemanticError("parameter type cannot be void", line, column);
                return false;
            }
            else if(id.empty())
            {
                // do not try an insert if the id is empty
            }
            else if(not scope->insert(id, type, true))
            {
                std::cout << RedefinitionError(id, line, column);
                return false;
            }
        }

        return true;
    }
};
