//============================================================================
// @author      : Thomas Dooms
// @date        : 3/21/20
// @copyright   : BA2 Informatica - Thomas Dooms - University of Antwerp
//============================================================================

#include "statements.h"
#include "IRVisitor/irVisitor.h"
#include "helper.h"
#include <numeric>

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
} // namespace

namespace Ast
{
std::string Scope::name() const
{
    return "block";
}

std::string Scope::value() const
{
    return "";
}

std::vector<Node*> Scope::children() const
{
    return std::vector<Node*>(statements.begin(), statements.end());
}

std::string Scope::color() const
{
    return "#ceebe3"; // light green
}

Literal* Scope::fold()
{
    for(auto& child : statements) assign_fold(child);
    return nullptr;
}

void Scope::visit(IRVisitor& visitor)
{
    visitor.visitScope(*this);
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
        if(auto* elem = table->lookup(variable->name()))
        {
            if(elem->type.isConst())
            {
                elem->literal = res->literal;
            }
            expr = res;
        }
        else
            throw InternalError("declaration variable not in table when folding");
    }

    // precast if it is constant
    if(expr->constant())
    {
        const auto lambda
        = [&](const auto& val) { return Helper::fold_cast(val, vartype, table, line, column); };

        if(auto* res = dynamic_cast<Literal*>(expr))
        {
            expr = std::visit(lambda, res->literal);
        }
    }

    return nullptr;
}

bool Declaration::check() const
{
    auto inserted = table->insert(variable->name(), vartype, expr);
    if(not inserted)
    {
        std::cout << RedefinitionError(variable->name(), line, column);
        return false;
    }

    if(vartype.isVoidType())
    {
        std::cout << SemanticError("type declaration cannot have void type");
        return false;
    }

    if(expr)
    {
        if(table->getType() == ScopeType::global and not expr->constant())
        {
            std::cout << NonConstantGlobal(variable->name(), line, column);
            return false;
        }
        return Type::convert(expr->type(), variable->type(), false, line, column);
    }
    else
        return true;
}

std::string FunctionDefinition::name() const
{
    return "function declaration";
}

std::string FunctionDefinition::value() const
{
    std::string res = returnType.string() + ' ' + identifier + '(';
    for(const auto& elem : parameters)
    {
        res += elem.first.string() + " " + elem.second;
    }
    return res + ')';
}

std::vector<Node*> FunctionDefinition::children() const
{
    return { body };
}

Literal* FunctionDefinition::fold()
{
    [[maybe_unused]] const auto _ = body->fold();
    return nullptr;
}

bool FunctionDefinition::check() const
{
    for(const auto& elem : parameters)
    {
        if(elem.first.isVoidType())
        {
            std::cout << SemanticError("parameter type cannot be void", line, column);
            return false;
        }
        if(not body->table->insert(elem.second, elem.first, true))
        {
            std::cout << RedefinitionError(identifier, line, column);
            return false;
        }
    }
    // don't look at me, i'm not the one leaking memory
    std::vector<Type*> types(parameters.size());
    const auto         convert = [&](const auto& param) { return new Type(param.first); };
    std::transform(parameters.begin(), parameters.end(), types.begin(), convert);

    const auto inserted = table->insert(identifier, Type(new Type(returnType), std::move(types)), true);
    if(not inserted)
    {
        std::cout << RedefinitionError(identifier, line, column);
        return false;
    }

    bool found = false;
    std::function<void(Node*)> func = [&](auto* root)
    {
      for(auto* child : root->children())
      {
          if(auto* res = dynamic_cast<ReturnStatement*>(child))
          {
              found = true;
              auto type = (res->expr) ? res->expr->type() : Type();
              const auto worked = Type::convert(type, returnType, false, line, column, true);
              if(not worked) return false;
          }
          func(child);
      }
      return true;
    };
    func(body);

    if(not found and not returnType.isVoidType())
    {
        std::cout << SemanticError("no return statement in nonvoid function", line, column, true);
    }

    return true;
}

void FunctionDefinition::visit(IRVisitor& visitor)
{
    visitor.visitFunctionDefinition(*this);
}

std::string FunctionDeclaration::name() const
{
    return "function declaration";
}

std::string FunctionDeclaration::value() const
{
    std::string res = returnType.string() + ' ' + identifier + '(';
    for(const auto& elem : parameters)
    {
        res += elem.string() + " ";
    }
    res.back() = ')';
    return res;
}

std::vector<Node*> FunctionDeclaration::children() const
{
    return {};
}

Literal* FunctionDeclaration::fold()
{
    return nullptr;
}

void FunctionDeclaration::visit(IRVisitor& visitor)
{

}

void Declaration::visit(IRVisitor& visitor)
{
    visitor.visitDeclaration(*this);
}

std::string LoopStatement::name() const
{
    if(doWhile) return "do while";
    else
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

void LoopStatement::visit(IRVisitor& visitor)
{
    visitor.visitLoopStatement(*this);
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
    if(ifBody) return { condition, ifBody };
    else
        return { condition, ifBody, elseBody };
}

Literal* IfStatement::fold()
{
    for(auto& child : children()) assign_fold(child);
    return nullptr;
}

void IfStatement::visit(IRVisitor& visitor)
{
    visitor.visitIfStatement(*this);
}

std::string ControlStatement::name() const
{
    return type;
}

std::string ControlStatement::value() const
{
    return "";
}

std::vector<Node*> ControlStatement::children() const
{
    return {};
}

Literal* ControlStatement::fold()
{
    return nullptr;
}

bool ControlStatement::check() const
{
    if(table->lookupType(ScopeType::loop))
    {
        return true;
    }
    else
    {
        std::cout << SemanticError(type + " statement is not in a loop", line, column);
        return false;
    }
}

std::string ReturnStatement::name() const
{
    return "return";
}

std::string ReturnStatement::value() const
{
    return "";
}

std::vector<Node*> ReturnStatement::children() const
{
    if(expr) return { expr };
    else return {};
}

Literal* ReturnStatement::fold()
{
    return nullptr;
}

bool ReturnStatement::check() const
{
    if(table->lookupType(ScopeType::function))
    {
        return true;
    }
    else
    {
        std::cout << SemanticError("return statement is not in a loop", line, column);
        return false;
    }
}

void ReturnStatement::visit(IRVisitor& visitor)
{
    visitor.visitReturnStatement(*this);
}

void ControlStatement::visit(IRVisitor& visitor)
{
    visitor.visitControlStatement(*this);
}

} // namespace Ast