//============================================================================
// @author      : Thomas Dooms
// @date        : 3/21/20
// @copyright   : BA2 Informatica - Thomas Dooms - University of Antwerp
//============================================================================

#include "statements.h"


namespace Ast
{
std::string Statement::color() const
{
    return " #ebcee5"; // light orange/pink
}

std::string Declaration::name() const
{
    return "declaration";
}

std::string Declaration::value() const
{
    return "";
}

std::vector<Node*> Declaration::children() const
{
    if(expr) return { variable, expr };
    else
        return { variable };
}

Literal* Declaration::fold()
{
    if(not expr) return nullptr;

    if(auto* res = expr->fold())
    {
        if(table->lookup_const(variable->name()))
        {
            table->set_literal(variable->name(), res->literal);
        }
        expr = res;
    }
    return nullptr;
}

bool Declaration::check() const
{
    const auto [iter, inserted] = table->insert(variable->name(), vartype, expr);
    if(not inserted)
    {
        std::cout << RedefinitionError(variable->name(), line, column);
        return false;
    }

    if(expr) return Type::convert(expr->type(), variable->type(), false, line, column);
    else
        return true;
}


std::string PrintfStatement::name() const
{
    return "printf";
}

std::string PrintfStatement::value() const
{
    return "";
}

std::vector<Node*> PrintfStatement::children() const
{
    return { expr };
}

Literal* PrintfStatement::fold()
{
    if(auto* res = expr->fold()) expr = res;
    return nullptr;
}
} // namespace Ast