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
    else
        return {};
}

Node* VariableDeclaration::fold()
{
    Helper::folder(expr);
    if(auto* res = dynamic_cast<Ast::Literal*>(expr))
    {
        const auto& entry = table->lookup(identifier);
        if(entry and entry->type->isConst())
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
        if(table->getType() == ScopeType::global)
        {
            if(table->lookup(identifier)->isInitialized)
            {
                std::cout
                << SemanticError("redefinition of already defined variable in global scope");
                return false;
            }
            else
            {
                table->lookup(identifier)->isInitialized &= static_cast<bool>(expr);
            }

            if(table->lookup(identifier)->type != type)
            {
                std::cout
                << SemanticError("redefinition of variable with different type in global scope", line, column);
            }
        }
        else
        {
            std::cout << RedefinitionError(identifier, line, column);
            return false;
        }
    }
    return true;
}

bool VariableDeclaration::check() const
{
    if(type->isVoidType())
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
    {
        return true;
    }
}

std::string FunctionDefinition::name() const
{
    return "function declaration";
}

std::string FunctionDefinition::value() const
{
return table->lookup(identifier)->type->string();
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
    auto res = Helper::fill_table_with_function(parameters, returnType, identifier, table, body->table, line, column);
    table->lookup(identifier)->isInitialized = true;
    return res;
}

bool FunctionDefinition::check() const
{
    if(identifier == "main") return true;
    if(returnType->isVoidType()) return true;

    bool                       found = false;
    std::function<void(Node*)> func  = [&](auto* root) {
        for(auto* child : root->children())
        {
            if(auto* res = dynamic_cast<ReturnStatement*>(child))
            {
                found             = true;
                auto       type   = (res->expr) ? res->expr->type() : new Type;
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
    return table->lookup(identifier)->type->string();
}

Node* FunctionDeclaration::fold()
{
    return this;
}

bool FunctionDeclaration::fill() const
{
    return Helper::fill_table_with_function(parameters, returnType, identifier, table,
                                     std::make_shared<SymbolTable>(ScopeType::plain, table), line, column);
}

void FunctionDeclaration::visit(IRVisitor& visitor) {}

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
        else if(elseBody)
            return elseBody;
        else
            return nullptr;
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

Node* ControlStatement::fold()
{
    return this;
}

void ControlStatement::visit(IRVisitor& visitor)
{
    visitor.visitControlStatement(*this);
}

std::string ReturnStatement::name() const
{
    return "return";
}

std::vector<Node*> ReturnStatement::children() const
{
    if(expr) return { expr };
    else
        return {};
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

std::string IncludeStdioStatement::name() const
{
    return "#include <stdio.h>";
}

std::vector<Node*> IncludeStdioStatement::children() const
{
    return {};
}

bool IncludeStdioStatement::check() const
{
    return true;
}

bool IncludeStdioStatement::fill() const
{
    auto returnType = new Type(false, BaseType::Int);
    auto strType = new Type(true, new Type(false, BaseType::Char));
    auto funcType = new Type(returnType, {strType}, true);

    table->insert("printf", funcType, false);
}

Node* IncludeStdioStatement::fold()
{
    return this;
}

void IncludeStdioStatement::visit(IRVisitor& visitor)
{
}

} // namespace Ast