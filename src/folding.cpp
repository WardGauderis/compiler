//============================================================================
// @name        : folding.h
// @author      : Thomas Dooms
// @date        : 3/10/20
// @version     :
// @copyright   : BA1 Informatica - Thomas Dooms - University of Antwerp
// @description :
//============================================================================

#include "folding.h"

namespace
{
template <typename Type0, typename Type1>
Ast::Literal* foldModulo(Type0 rhs, Type1 lhs)
{
    if constexpr (std::is_integral_v<Type0> and std::is_integral_v<Type1>)
    {
        return new Ast::Literal(rhs % lhs);
    }
    else
    {
        throw WhoopsiePoopsieError("Someone should already have checked if "
                                   "modulo is done on floating point before");
    }
}

template <typename Type0, typename Type1>
Ast::Literal* foldBinary(Type0 rhs, Type1 lhs, const std::string& operation)
{
    if ((operation == "/" or operation == "%") and rhs == 0) return nullptr;

    if (operation == "+")
        return new Ast::Literal(lhs + rhs);
    else if (operation == "-")
        return new Ast::Literal(lhs - rhs);
    else if (operation == "*")
        return new Ast::Literal(lhs * rhs);
    else if (operation == "/")
        return new Ast::Literal(lhs / rhs);
    else if (operation == "%")
        return foldModulo(lhs, rhs);
    else if (operation == "<")
        return new Ast::Literal(lhs < rhs);
    else if (operation == ">")
        return new Ast::Literal(lhs > rhs);
    else if (operation == "<=")
        return new Ast::Literal(lhs <= rhs);
    else if (operation == ">=")
        return new Ast::Literal(lhs >= rhs);
    else if (operation == "==")
        return new Ast::Literal(lhs == rhs);
    else if (operation == "!=")
        return new Ast::Literal(lhs != rhs);
    else if (operation == "&&")
        return new Ast::Literal(lhs && rhs);
    else if (operation == "||")
        return new Ast::Literal(lhs || rhs);
    else
        throw SyntaxError("unknown binary operation");
}

template <typename Type>
Ast::Literal* foldUnary(Type rhs, const std::string& operation)
{
    if (operation == "+")
        return new Ast::Literal(rhs);
    else if (operation == "-")
        return new Ast::Literal(-rhs);
    else if (operation == "!")
        return new Ast::Literal(!rhs);
    else
        throw SyntaxError("unknown unary operation");
}

}

Ast::Expr* foldExpr(Ast::Expr* expr)
{
    downcast_expr(expr, overloaded {
        [&](Ast::BinaryExpr* binexpr) -> Ast::Expr*
        {
          binexpr->lhs = foldExpr(binexpr->lhs);
          binexpr->rhs = foldExpr(binexpr->rhs);

          auto* lhs = dynamic_cast<Ast::Literal*>(binexpr->lhs);
          auto* rhs = dynamic_cast<Ast::Literal*>(binexpr->rhs);

          if(lhs == nullptr or rhs == nullptr) return expr;

          const auto lambda = [&](const auto& lhs, const auto& rhs)
          {
            auto* res = foldBinary(lhs, rhs, binexpr->operation);
            if(res == nullptr) return expr;
            else return static_cast<Ast::Expr*>(res);
          };
          return std::visit(lambda, lhs->literal, rhs->literal);
        },
        [&](Ast::UnaryExpr* unexpr) -> Ast::Expr*
        {
          unexpr->operand = foldExpr(unexpr->operand);
          auto* operand = dynamic_cast<Ast::Literal*>(unexpr->operand);
          if(operand == nullptr) return expr;

          const auto lambda = [&](const auto& val)
          {
            return foldUnary(val, unexpr->operation);
          };
          return std::visit(lambda, operand->literal);
        },
        [&](auto*)
        {
          return expr;
        }});
}

std::unique_ptr<Ast::Block> foldNodes(const std::vector<Ast::Node*>& nodes)
{
    for(auto& child : nodes)
    {
        downcast_node(child, overloaded
            {
                [&](Ast::Declaration* declaration)
                {
                    if(auto res = foldExpr(declaration->expr))
                        declaration->expr = res;
                },
                [&](Ast::ExprStatement* statement)
                {
                  if(auto res = foldExpr(statement->expr))
                      statement->expr = res;
                },
                [&](Ast::Comment* comment)
                {
                },
                [&](auto*)
                {
                  throw WhoopsiePoopsieError("too lazy, let's see what happens");
                }
            });
    }
    return std::make_unique<Ast::Block>(nodes);
}









