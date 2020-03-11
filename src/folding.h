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

Expr* foldExpr(Expr* expr)
{
    Expr* result = nullptr;
    downcast_call(expr, overloaded
    (
        [&](BinaryExpr* binexpr)
        {
            if(binexpr->lhs->get_id() != Int::ID)
            {
                binexpr->lhs = foldExpr(binexpr->lhs);
            }
            if(binexpr->rhs->get_id() != Int::ID)
            {
                binexpr->rhs = foldExpr(binexpr->rhs);
            }

            if(binexpr->lhs->get_id() != Int::ID
            or binexpr->rhs->get_id() != Int::ID) return;

            const auto lhs = static_cast<Int*>(binexpr->lhs)->val;
            const auto rhs = static_cast<Int*>(binexpr->rhs)->val;

            if((binexpr->operation == "/"
            or binexpr->operation == "%")
            and rhs == 0) return;

            if      (binexpr->operation == "+" ) result = new Int(lhs + rhs);
            else if (binexpr->operation == "-" ) result = new Int(lhs - rhs);
            else if (binexpr->operation == "*" ) result = new Int(lhs * rhs);
            else if (binexpr->operation == "/" ) result = new Int(lhs / rhs);
            else if (binexpr->operation == "%" ) result = new Int(lhs % rhs);
            else if (binexpr->operation == "<" ) result = new Int(lhs < rhs);
            else if (binexpr->operation == ">" ) result = new Int(lhs > rhs);
            else if (binexpr->operation == "<=") result = new Int(lhs <= rhs);
            else if (binexpr->operation == ">=") result = new Int(lhs >= rhs);
            else if (binexpr->operation == "==") result = new Int(lhs == rhs);
            else if (binexpr->operation == "!=") result = new Int(lhs != rhs);
            else if (binexpr->operation == "&&") result = new Int(lhs && rhs);
            else if (binexpr->operation == "||") result = new Int(lhs || rhs);
            else throw std::logic_error("unknown binary operation");
        },
        [&](UnaryExpr* unexpr)
        {
            if(unexpr->operand->get_id() != Int::ID)
            {
                unexpr->operand = foldExpr(unexpr->operand);
            }

            if(unexpr->operand->get_id() != Int::ID) return;
            const auto val = static_cast<Int*>(unexpr->operand)->val;

            if      (unexpr->operation == "+") result = new Int(val);
            else if (unexpr->operation == "-") result = new Int(-val);
            else if (unexpr->operation == "!") result = new Int(!val);
            else throw std::logic_error("unknown unary operation");
        },
        [&](auto) {}
        ));

    if(result == nullptr) return expr;
    else return result;
}

void foldFile(std::unique_ptr<File>& root)
{
    for(auto& child : root->expressions)
    {
        child = foldExpr(child);
    }
}









