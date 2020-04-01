//============================================================================
// @author      : Thomas Dooms
// @date        : 3/21/20
// @copyright   : BA2 Informatica - Thomas Dooms - University of Antwerp
//============================================================================

#include "expressions.h"
#include "helper.h"
#include <IRVisitor/irVisitor.h>


namespace Ast
{
std::string Expr::color() const
{
    return "#ced6eb"; // light blue
}

std::string Comment::name() const
{
    return "comment";
}

std::string Comment::value() const
{
    return "...";
}


std::string Comment::color() const
{
    return "#d5ceeb"; // light purple
}


void Comment::visit(IRVisitor& visitor)
{
    visitor.visitComment(*this);
}

std::string Literal::name() const
{
    return "literal";
}

std::string Literal::value() const
{
    return std::visit([&](const auto& val) { return std::to_string(val); }, literal);
}


Literal* Literal::fold()
{
    return this;
}

Type Literal::type() const
{
    return Type(true, static_cast<BaseType>(literal.index()));
}

bool Literal::constant() const
{
    return true;
}

void Literal::visit(IRVisitor& visitor)
{
    visitor.visitLiteral(*this);
}

std::string Variable::name() const
{
    return identifier;
}

std::string Variable::value() const
{
    return table->lookup(identifier)->type.string();
}

std::string Variable::color() const
{
    return "#ebe6ce";
}

Literal* Variable::fold()
{
    if(auto* res = table->lookup(identifier))
    {
        if(not res->literal.has_value()) return nullptr;
        else
            return new Ast::Literal(res->literal.value(), table, line, column);
    }
    throw InternalError("variable not found while folding");
}

bool Variable::check() const
{
    if(auto* res = table->lookup(identifier))
    {
        if(not res->isInitialized)
        {
            std::cout << UninitializedWarning(identifier, line, column);
            res->isInitialized = true;
        }
    }
    else
    {
        std::cout << UndeclaredError(identifier, line, column);
        return false;
    }
    return true;
}

Type Variable::type() const
{
    if(auto* res = table->lookup(identifier)) return res->type;
    else
        return Type();
}

bool Variable::constant() const
{
    return false;
}

void Variable::visit(IRVisitor& visitor)
{
    visitor.visitVariable(*this);
}

std::string BinaryExpr::name() const
{
    return "binary expression";
}

std::string BinaryExpr::value() const
{
    return operation.string();
}

std::vector<Node*> BinaryExpr::children() const
{
    return { lhs, rhs };
}

Literal* BinaryExpr::fold()
{
    lhs = Helper::folder(lhs);
    rhs = Helper::folder(rhs);

    const auto res0 = dynamic_cast<Literal*>(lhs);
    const auto res1 = dynamic_cast<Literal*>(rhs);

    if(res0 and res1)
    {
        const auto lambda = [&](const auto& val0, const auto& val1)
        {
            return Helper::fold_binary(val0, val1, operation, table, line, column);
        };
        return std::visit(lambda, res0->literal, res1->literal);
    }
    return nullptr;
}

bool BinaryExpr::check() const
{
    return Type::combine(operation, lhs->type(), rhs->type(), line, column).has_value();
}

Type BinaryExpr::type() const
{
    try
    {
        return Type::combine(operation, lhs->type(), rhs->type(), 0, 0, false).value();
    }
    catch(...)
    {
        return Type();
    }
}
bool BinaryExpr::constant() const
{
    return rhs->constant() && lhs->constant();
}

void BinaryExpr::visit(IRVisitor& visitor)
{
    visitor.visitBinaryExpr(*this);
}

std::string PrefixExpr::name() const
{
    return "prefix expression";
}

std::string PrefixExpr::value() const
{
    return operation.string() + operand->value();
}

std::vector<Node*> PrefixExpr::children() const
{
    return { operand };
}

Literal* PrefixExpr::fold()
{
    operand = Helper::folder(operand);

    if(auto* res = dynamic_cast<Ast::Literal*>(operand))
    {
        const auto lambda = [&](const auto& val) { return Helper::fold_prefix(val, operation, table, line, column); };
        return std::visit(lambda, res->literal);
    }
    return nullptr;
}

bool PrefixExpr::check() const
{
    if(operation.isIncrDecr() and operand->type().isConst())
    {
        std::cout << ConstError("prefix operator", operand->name(), line, column);
        return false;
    }

    if(not Helper::is_lvalue(operand))
    {
        if(operation.isIncrDecr())
        {
            std::cout << RValueError("assigning to", line, column);
            return false;
        }
    }

    return Type::unary(operation, operand->type(), line, column).has_value();
}

Type PrefixExpr::type() const
{
    try
    {
        return Type::unary(operation, operand->type(), 0, 0, false).value();
    }
    catch(...)
    {
        return Type();
    }
}

bool PrefixExpr::constant() const
{
    return operand->constant();
}

void PrefixExpr::visit(IRVisitor& visitor)
{
    visitor.visitPrefixExpr(*this);
}

std::string PostfixExpr::name() const
{
    return "prefix expression";
}

std::string PostfixExpr::value() const
{
    return operation.string() + operand->value();
}

std::vector<Node*> PostfixExpr::children() const
{
    return { operand };
}

Literal* PostfixExpr::fold()
{
    operand = Helper::folder(operand);

    auto*      res = dynamic_cast<Ast::Literal*>(operand);
    const auto lambda
    = [&](const auto& val) { return Helper::fold_postfix(val, operation, table, line, column); };

    // TODO: deletus feetus, memory leakus
    if(res) return std::visit(lambda, res->literal);
    return nullptr;
}

bool PostfixExpr::check() const
{
    if(not Helper::is_lvalue(operand))
    {
        std::cout << RValueError("assigning to", line, column);
        return false;
    }

    if(operand->type().isConst())
    {
        std::cout << ConstError("postfix expr", operand->name(), line, column);
        return false;
    }
    return true;
}

Type PostfixExpr::type() const
{
    return operand->type();
}
bool PostfixExpr::constant() const
{
    return false;
}

void PostfixExpr::visit(IRVisitor& visitor)
{
    visitor.visitPostfixExpr(*this);
}

std::string CastExpr::name() const
{
    return "cast expression";
}

std::string CastExpr::value() const
{
    return '(' + cast.string() + ')';
}

std::vector<Node*> CastExpr::children() const
{
    return { operand };
}

Literal* CastExpr::fold()
{
    operand = Helper::folder(operand);

    auto*      res = dynamic_cast<Ast::Literal*>(operand);
    const auto lambda
    = [&](const auto& val) { return Helper::fold_cast(val, cast, table, line, column); };

    if(res) return std::visit(lambda, res->literal);
    else
        return nullptr;
}

bool CastExpr::check() const
{
    return Type::convert(operand->type(), cast, true, line, column);
}

Type CastExpr::type() const
{
    return cast;
}
bool CastExpr::constant() const
{
    return operand->constant();
}

void CastExpr::visit(IRVisitor& visitor)
{
    visitor.visitCastExpr(*this);
}

std::string Assignment::name() const
{
    return "assignment";
}

std::vector<Node*> Assignment::children() const
{
    return { lhs, rhs };
}

Literal* Assignment::fold()
{
    rhs = Helper::folder(rhs);
    return nullptr;
}

bool Assignment::check() const
{
    if(not Helper::is_lvalue(lhs))
    {
        std::cout << RValueError("assigning to", line, column);
        return false;
    }

    if(auto* res = table->lookup(lhs->name()))
    {
        if(res->type.isConst())
        {
            std::cout << ConstError("assignment", lhs->name(), line, column);
            return false;
        }
        table->lookup(lhs->name())->isInitialized = true;
    }

    return Type::convert(rhs->type(), lhs->type(), false, line, column);
}

Type Assignment::type() const
{
    return lhs->type();
}
bool Assignment::constant() const
{
    return false;
}

void Assignment::visit(IRVisitor& visitor)
{
    visitor.visitAssignment(*this);
}

std::string PrintfStatement::name() const
{
    return "printf";
}

std::string FunctionCall::name() const
{
    return "function call";
}

std::string FunctionCall::value() const
{
    return identifier;
}

std::vector<Node*> FunctionCall::children() const
{
    return std::vector<Node*>(arguments.begin(), arguments.end());
}

Literal * FunctionCall::fold()
{
    for(auto& child : arguments) child = Helper::folder(child);
    return nullptr;
}

bool FunctionCall::check() const
{
    if(auto* res = table->lookup(identifier))
    {
        if(not res->type.isFunctionType())
        {
            std::cout << SemanticError("calling non function object: " + identifier, line, column);
            return false;
        }

        const auto& func = res->type.getFunctionType();
        if(func.second.size() != arguments.size())
        {
            std::cout << WrongArgumentCount(identifier, func.second.size(), arguments.size(), line, column);
            return false;
        }

        bool error = false;
        for(size_t i = 0; i < arguments.size(); i++)
        {
            error &= Type::convert(arguments[i]->type(), *func.second[i], false, line, column, true);
        }
        return not error;
    }
    else
    {
        std::cout << UndeclaredError(identifier, line, column);
        return false;
    }
}

Type FunctionCall::type() const
{
    if(auto* res = table->lookup(identifier))
    {
        return *res->type.getFunctionType().first;
    }
    else
    {
        throw InternalError("function with name: " + identifier + " not found in table");
    }
}

bool FunctionCall::constant() const
{
    return false;
}

void FunctionCall::visit(IRVisitor& visitor)
{
    visitor.visitFunctionCall(*this);
}

std::vector<Node*> PrintfStatement::children() const
{
    return { expr };
}

Literal* PrintfStatement::fold()
{
    expr = Helper::folder(expr);
    return nullptr;
}

Type PrintfStatement::type() const
{
    return Type(false, BaseType::Int);
}

bool PrintfStatement::constant() const
{
    return false;
}

void PrintfStatement::visit(IRVisitor& visitor)
{
    visitor.visitPrintfStatement(*this);
}
} // namespace Ast