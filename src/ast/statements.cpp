//============================================================================
// @author      : Thomas Dooms
// @date        : 3/21/20
// @copyright   : BA2 Informatica - Thomas Dooms - University of Antwerp
//============================================================================

#include "statements.h"

namespace
{
template <typename Type>
bool assign_fold(Type*& elem)
{
    if(auto* res = elem->fold())
    {
        elem = res;
        return true;
    }
    return false;
}
}

namespace Ast
{
std::string Scope::name () const
{
    return "block";
}

std::string Scope::value () const
{
    return "";
}

std::vector<Node*> Scope::children () const
{
    return std::vector<Node*>(statements.begin(), statements.end());
}

std::string Scope::color () const
{
    return "#ceebe3"; // light green
}

Literal* Scope::fold ()
{
    for (auto& child : statements) assign_fold(child);
    return nullptr;
}

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

std::string LoopStatement::name() const
{
    return "loop";
}

std::string LoopStatement::value() const
{
    return "";
}

std::vector<Node*> LoopStatement::children() const
{
    std::vector<Node*> res;
    if(init) res.emplace_back(init);
    if(condition) res.emplace_back(condition);
    if(iteration) res.emplace_back(iteration);
    res.emplace_back(body);
    return res;
}

Literal* LoopStatement::fold()
{
    for(auto& child : children()) assign_fold(child);
    return nullptr;
}

std::string IfStatement::name() const
{
    return "if";
}

std::string IfStatement::value() const
{
    return "";
}

std::vector<Node*> IfStatement::children() const
{
    if (ifBody) return {condition, ifBody};
    else return {condition, ifBody, elseBody};
}

Literal* IfStatement::fold()
{
    for(auto& child : children()) assign_fold(child);
    return nullptr;
}

std::string controlStatement::name() const
{
    return type;
}

std::string controlStatement::value() const
{
    return "";
}

std::vector<Node*> controlStatement::children() const
{
    return {};
}

Literal* controlStatement::fold()
{
    return nullptr;
}

} // namespace Ast