//============================================================================
// @name        : folding.h
// @author      : Thomas Dooms
// @date        : 3/10/20
// @version     :
// @copyright   : BA1 Informatica - Thomas Dooms - University of Antwerp
// @description :
//============================================================================

#pragma once

#include "ast.h"

Ast::Expr* foldExpr(Ast::Expr* expr)
{
    Ast::Expr* result = nullptr;
    downcast_call(expr, overloaded{
         [&](Ast::BinaryExpr* binexpr){
         auto* lhs = dynamic_cast<Ast::Literal*>(binexpr->lhs);
         auto* rhs = dynamic_cast<Ast::Literal*>(binexpr->rhs);

         bool foldLhs = true;
         if (lhs == nullptr)
         {
             auto* res = foldExpr(lhs);
             if (res != nullptr)
                 binexpr->lhs = res;
             else
                 foldLhs = false;
         }

         bool foldRhs = true;
         if (rhs == nullptr)
         {
             auto* res = foldExpr(rhs);
             if (res != nullptr)
                 binexpr->rhs = res;
             else
                 foldRhs = false;
         }
         if (not(foldLhs and foldRhs)) return;

         const auto val0 = static_cast<Ast::Literal*>(binexpr->lhs)->literal;
         const auto val1 = static_cast<Ast::Literal*>(binexpr->rhs)->literal;

         std::visit(
             [&](const auto& lhs, const auto& rhs) {
                 if ((binexpr->operation == "/" or binexpr->operation == "%") and rhs == 0)
                     return;

                 if (binexpr->operation == "+")
                     result = new Ast::Literal(lhs + rhs);
                 else if (binexpr->operation == "-")
                     result = new Ast::Literal(lhs - rhs);
                 else if (binexpr->operation == "*")
                     result = new Ast::Literal(lhs * rhs);
                 else if (binexpr->operation == "/")
                     result = new Ast::Literal(lhs / rhs);
                 //                else if (binexpr->operation == "%" ) result = new Ast::Literal(lhs %  rhs);
                 else if (binexpr->operation == "<")
                     result = new Ast::Literal(lhs < rhs);
                 else if (binexpr->operation == ">")
                     result = new Ast::Literal(lhs > rhs);
                 else if (binexpr->operation == "<=")
                     result = new Ast::Literal(lhs <= rhs);
                 else if (binexpr->operation == ">=")
                     result = new Ast::Literal(lhs >= rhs);
                 else if (binexpr->operation == "==")
                     result = new Ast::Literal(lhs == rhs);
                 else if (binexpr->operation == "!=")
                     result = new Ast::Literal(lhs != rhs);
                 else if (binexpr->operation == "&&")
                     result = new Ast::Literal(lhs && rhs);
                 else if (binexpr->operation == "||")
                     result = new Ast::Literal(lhs || rhs);
                 else
                     throw std::logic_error("unknown binary operation");
             },
             val0, val1);
     },
     [&](Ast::UnaryExpr* unexpr) {
         auto* operand = dynamic_cast<Ast::Literal*>(unexpr->operand);
         if (operand == nullptr)
         {
             auto* res = foldExpr(operand);
             if (res == nullptr)
                 return nullptr;
             else
                 unexpr->operand = res;
         }
         const auto val0 = static_cast<Ast::Literal*>(unexpr->operand)->literal;

         std::visit(
             [&](const auto& val) {
                 if (unexpr->operation == "+")
                     result = new Ast::Literal(val);
                 else if (unexpr->operation == "-")
                     result = new Ast::Literal(-val);
                 else if (unexpr->operation == "!")
                     result = new Ast::Literal(!val);
                 else
                     throw std::logic_error("unknown unary operation");
             },
             val0);
     },
     [&](auto) {}});
    return result;
}

void foldFile(std::unique_ptr<Ast::Block>& root)
{
    for(auto& child : root->expressions)
    {
        child = foldExpr(child);
    }
}









