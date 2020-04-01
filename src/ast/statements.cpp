//============================================================================
// @author      : Thomas Dooms
// @date        : 3/21/20
// @copyright   : BA2 Informatica - Thomas Dooms - University of Antwerp
//============================================================================

#include "statements.h"
#include "IRVisitor/irVisitor.h"
#include "helper.h"
#include <numeric>

namespace Ast
{
std::string Scope::name() const
{
    return "block";
}

std::vector<Node*> Scope::children() const
{
    return std::vector<Node*>(statements.begin(), statements.end());
}

std::string Scope::color() const
{
    return "#ceebe3"; // light green
}
Literal * Scope::fold()
{
    for(auto& child : statements) child = Helper::folder(child);
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

std::vector<Node*> Declaration::children() const
{
    if(expr) return { variable, expr };
    else return { variable };
}

Literal* Declaration::fold()
{
    if(not expr) return nullptr;

    expr = Helper::folder(expr);
    if(auto* res = dynamic_cast<Ast::Literal*>(expr))
    {
        const auto& entry = table->lookup(variable->name());
        if(entry and entry->type.isConst())
        {
            entry->literal = res->literal;
        }
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

bool Declaration::fill() const
{
    if(not table->insert(variable->name(), vartype, expr))
    {
        std::cout << RedefinitionError(variable->name(), line, column);
        return false;
    }
    return true;
}

bool Declaration::check() const
{
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

Node* FunctionDefinition::fold()
{
    body = Helper::folder(body);
    return nullptr;
}

bool FunctionDefinition::fill() const
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

    std::vector<Type*> types(parameters.size());
    const auto         convert = [&](const auto& param) { return new Type(param.first); };
    std::transform(parameters.begin(), parameters.end(), types.begin(), convert);

    const auto inserted = table->insert(identifier, Type(new Type(returnType), std::move(types)), true);
    if(not inserted)
    {
        std::cout << RedefinitionError(identifier, line, column);
        return false;
    }
    return true;
}

bool FunctionDefinition::check() const
{
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

Node* FunctionDeclaration::fold()
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

std::vector<Node*> LoopStatement::children() const
{
    std::vector<Node*> res;
    if(init) res.emplace_back(init);
    if(condition) res.emplace_back(condition);
    if(iteration) res.emplace_back(iteration);
    res.emplace_back(body);
    return res;
}

Node* LoopStatement::fold()
{
    if(init) init = Helper::folder(init);
    if(condition) condition = Helper::folder(condition);
    if(iteration) iteration = Helper::folder(iteration);
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

std::vector<Node*> IfStatement::children() const
{
    std::vector<Node*> res;
    if(condition) res.emplace_back(condition);
    if(ifBody) res.emplace_back(ifBody);
    if(elseBody) res.emplace_back(elseBody);
    return res;
}

Node* IfStatement::fold()
{
    condition = Helper::folder(condition);

    if(auto* res = dynamic_cast<Ast::Literal*>(condition))
    {
        if(Helper::evaluate(res)) return ifBody;
        else if(elseBody) return elseBody;
        else return new Scope({}, table, line, column);
    }

    ifBody = Helper::folder(ifBody);
    if(elseBody) elseBody = Helper::folder(elseBody);

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

Node * ControlStatement::fold()
{
    return nullptr;
}

std::string ReturnStatement::name() const
{
    return "return";
}

std::vector<Node*> ReturnStatement::children() const
{
    if(expr) return { expr };
    else return {};
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

Node* ReturnStatement::fold()
{
    expr = Helper::folder(expr);
    return nullptr;
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