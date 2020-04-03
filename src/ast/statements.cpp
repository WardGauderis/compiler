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
Node* Scope::fold()
{
    for(auto iter = statements.begin(); iter != statements.end();)
    {
        if(Helper::folder(*iter))
        {
            iter = statements.erase(iter);
        }
        else
        {
            iter++;
        }
    }
    return this;
}

void Scope::visit(IRVisitor& visitor)
{
    visitor.visitScope(*this);
}

std::string Statement::color() const
{
    return " #ebcee5"; // light orange/pink
}

std::string VariableDeclaration::name() const
{
    return "variable declaration";
}

std::vector<Node*> VariableDeclaration::children() const
{
    if(expr) return { expr };
    else return { };
}

Node* VariableDeclaration::fold()
{
    Helper::folder(expr);
    if(auto* res = dynamic_cast<Ast::Literal*>(expr))
    {
        const auto& entry = table->lookup(identifier);
        if(entry and entry->type.isConst())
        {
            entry->literal = res->literal;
        }
    }

    // precast if it is constant
    if(expr and expr->constant())
    {
        const auto lambda
        = [&](const auto& val) { return Helper::fold_cast(val, type, table, line, column); };

        if(auto* res = dynamic_cast<Literal*>(expr))
        {
            expr = std::visit(lambda, res->literal);
        }
    }

    return this;
}

bool VariableDeclaration::fill() const
{
    if(not table->insert(identifier, type, expr))
    {
        std::cout << RedefinitionError(identifier, line, column);
        return false;
    }
    return true;
}

bool VariableDeclaration::check() const
{
    if(type.isVoidType())
    {
        std::cout << SemanticError("type declaration cannot have void type");
        return false;
    }

    if(expr)
    {
        if(table->getType() == ScopeType::global and not expr->constant())
        {
            std::cout << NonConstantGlobal(identifier, line, column);
            return false;
        }
        return Type::convert(expr->type(), type, false, line, column);
    }
    else
        return true;
}

std::string ArrayDeclaration::name() const
{
    return "array declaration";
}

std::vector<Node*> ArrayDeclaration::children() const
{
    return {};
}

Node* ArrayDeclaration::fold()
{
    return this;
}

bool ArrayDeclaration::fill() const
{
    if(not table->insert(identifier, type, false))
    {
        std::cout << RedefinitionError(identifier, line, column);
        return false;
    }
    return true;
}

bool ArrayDeclaration::check() const
{
    if(type.isVoidType())
    {
        std::cout << SemanticError("type declaration cannot have void type");
        return false;
    }
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
    Helper::folder(body);
    return this;
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
    if(identifier == "main") return true;
    if(returnType.isVoidType()) return true;

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

    if(not found)
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
    return this;
}

void FunctionDeclaration::visit(IRVisitor& visitor)
{

}

void VariableDeclaration::visit(IRVisitor& visitor)
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
    Helper::folder(init);
    Helper::folder(condition);
    Helper::folder(iteration);
    return this;
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
    Helper::folder(condition);

    if(auto* res = dynamic_cast<Ast::Literal*>(condition))
    {
        if(Helper::evaluate(res)) return ifBody;
        else if(elseBody) return elseBody;
        else return nullptr;
    }

    Helper::folder(ifBody);
    Helper::folder(elseBody);

    return this;
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
    return this;
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
    Helper::folder(expr);
    return this;
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