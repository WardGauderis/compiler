//============================================================================
// @author      : Thomas Dooms & Ward Gauderis
// @date        : 3/10/20
// @copyright   : BA2 Informatica - Thomas Dooms & Ward Gauderis - University of Antwerp
//============================================================================

#include "ast.h"
#include <fstream>
#include <functional>
#include <iostream>

namespace
{
template<typename Type>
bool assign_fold(Type*& elem)
{
    if (auto* res = elem->fold())
    {
        elem = res;
        return true;
    }
    return false;
}

template<typename Type0, typename Type1>
Ast::Literal* fold_modulo(Type0 lhs, Type1 rhs, std::shared_ptr<SymbolTable> table, size_t line, size_t column)
{
    if constexpr (std::is_integral_v<Type0> and std::is_integral_v<Type1>)
    {
        return new Ast::Literal(lhs % rhs, std::move(table), line, column);
    }
    else
    {
        throw InternalError("modulo on floating points while folding", line, column);
    }
}

template<typename Type0, typename Type1>
Ast::Literal* fold_binary(Type0 lhs, Type1 rhs, const std::string& operation, std::shared_ptr<SymbolTable> table, size_t line, size_t column)
{
    if ((operation == "/" or operation == "%") and rhs == 0) return nullptr;

    if (operation == "+") return new Ast::Literal(lhs + rhs, std::move(table), line, column);
    else if (operation == "-")
        return new Ast::Literal(lhs - rhs, std::move(table), line, column);
    else if (operation == "*")
        return new Ast::Literal(lhs * rhs, std::move(table), line, column);
    else if (operation == "/")
        return new Ast::Literal(lhs / rhs, std::move(table), line, column);
    else if (operation == "%")
        return fold_modulo(lhs, rhs, std::move(table), line, column);
    else if (operation == "<")
        return new Ast::Literal(lhs < rhs, std::move(table), line, column);
    else if (operation == ">")
        return new Ast::Literal(lhs > rhs, std::move(table), line, column);
    else if (operation == "<=")
        return new Ast::Literal(lhs <= rhs, std::move(table), line, column);
    else if (operation == ">=")
        return new Ast::Literal(lhs >= rhs, std::move(table), line, column);
    else if (operation == "==")
        return new Ast::Literal(lhs == rhs, std::move(table), line, column);
    else if (operation == "!=")
        return new Ast::Literal(lhs != rhs, std::move(table), line, column);
    else if (operation == "&&")
        return new Ast::Literal(lhs && rhs, std::move(table), line, column);
    else if (operation == "||")
        return new Ast::Literal(lhs || rhs, std::move(table), line, column);
    else
        throw SyntaxError("unknown binary operation", line, column);
}

template<typename Type>
Ast::Literal* fold_unary(Type operand, const std::string& operation, std::shared_ptr<SymbolTable> table, size_t line, size_t column)
{
    if (operation == "+") return new Ast::Literal(operand, std::move(table), line, column);
    else if (operation == "-")
        return new Ast::Literal(-operand, std::move(table), line, column);
    else if (operation == "!")
        return new Ast::Literal(!operand, std::move(table), line, column);
    else
        throw SyntaxError("unknown unary operation", line, column);
}

template<typename Type>
Ast::Literal* fold_cast(Type operand, const std::string& operation, std::shared_ptr<SymbolTable> table, size_t line, size_t column)
{
    if (operation == "float") return new Ast::Literal((float)operand, std::move(table), line, column);
    else if (operation == "double")
        return new Ast::Literal((double)operand, std::move(table), line, column);
    else if (operation == "char")
        return new Ast::Literal((char)operand, std::move(table), line, column);
    else if (operation == "short")
        return new Ast::Literal((short)operand, std::move(table), line, column);
    else if (operation == "int")
        return new Ast::Literal((int)operand, std::move(table), line, column);
    else if (operation == "long")
        return new Ast::Literal((long)operand, std::move(table), line, column);
    else
        throw InternalError("unknown type for conversion: " + operation, line, column);
}
} // namespace

namespace Ast {
std::ofstream& operator<<(std::ofstream& stream, const std::unique_ptr<Node>& root)
{
    stream << "digraph G\n";
    stream << "{\n";

    std::function<void(Node*)> recursion = [&](Node* node)
    {
      stream << '"' << node << "\"[label=\"" << node->name() << "\\n"
             << node->value() << "\", shape=box, style=filled, color=\"" << node->color() << "\"];\n";

      for (const auto child : node->children())
      {
          stream << '"' << node << "\" -> \"" << child << "\";\n";
          recursion(child);
      }
    };
    recursion(root.get());
    stream << "}\n";
    return stream;
}

void Node::complete(bool check, bool fold, bool output)
{
    std::function<void(Ast::Node*)> recursion = [&](Ast::Node* root)
    {
      if (check) root->check();
      for (const auto child : root->children())
      {
          recursion(child);
      }
    };
    recursion(this);

    if (fold) this->fold();
}

std::string Expr::color() const
{
    return "#ced6eb"; // light blue
}

std::string Statement::color() const
{
    return " #ebcee5"; // light orange/pink
}

std::string Comment::name() const
{
    return "comment";
}

std::string Comment::value() const
{
    return "...";
}

std::vector<Node*> Comment::children() const
{
    return {};
}

std::string Comment::color() const
{
    return "#d5ceeb"; // light purple
}

Literal* Comment::fold()
{
    return nullptr;
}

void Comment::check() const
{
}

std::string Block::name() const
{
    return "block";
}

std::string Block::value() const
{
    return "";
}

std::vector<Node*> Block::children() const
{
    return nodes;
}

std::string Block::color() const
{
    return "#ceebe3"; // light green
}

Literal* Block::fold()
{
    for (auto& child : nodes) assign_fold(child);
    return nullptr;
}

void Block::check() const
{
}

std::string Literal::name() const
{
    return "literal";
}

std::string Literal::value() const
{
    return std::visit([&](const auto& val)
                      { return std::to_string(val); }, literal);
}

std::vector<Node*> Literal::children() const
{
    return {};
}

Literal* Literal::fold()
{
    return this;
}

void Literal::check() const
{
}

Type Literal::type() const
{
    return Type(true, static_cast<BaseType>(literal.index()));
}

std::string Variable::name() const
{
    return entry->first;
}

std::string Variable::value() const
{
    return entry->second.type.string();
}

std::vector<Node*> Variable::children() const
{
    return {};
}

std::string Variable::color() const
{
    return "#ebe6ce";
}

Literal* Variable::fold()
{
    const auto& literal = table->get_literal(name());
    if (literal.has_value())
    {
        return new Ast::Literal(literal.value(), table, column , line);
    }
    else
        return nullptr;
}

void Variable::check() const
{
}

Type Variable::type() const
{
    return entry->second.type;
}

std::string BinaryExpr::name() const
{
    return "binary expression";
}

std::string BinaryExpr::value() const
{
    return operation;
}

std::vector<Node*> BinaryExpr::children() const
{
    return {lhs, rhs};
}

Literal* BinaryExpr::fold()
{
    auto* new_lhs = lhs->fold();
    auto* new_rhs = rhs->fold();

    const auto set_folded = [&]() -> Ast::Literal*
    {
      if (new_lhs) lhs = new_lhs;
      if (new_rhs) rhs = new_rhs;
      return nullptr;
    };

    if (new_lhs and new_rhs)
    {
        const auto lambda = [&](const auto& lhs, const auto& rhs)
        {
          auto* res = fold_binary(lhs, rhs, operation, table, line, column);
          if (res) return res;
          else
              return set_folded();
        };
        // TODO: deletus feetus, memory leakus
        return std::visit(lambda, new_lhs->literal, new_rhs->literal);
    }
    else
    {
        return set_folded();
    }
}

void BinaryExpr::check() const
{
    const auto rtype = rhs->type();
    const auto ltype = lhs->type();
    if (not rtype.isBaseType() or not ltype.isBaseType())
        std::cout << InternalError("pointer arithmetic not yet supported", line, column, true);

    if (operation=="%" and (rtype.getBaseType()==BaseType::Float or ltype.getBaseType()==BaseType::Float))
        std::cout << InternalError("modulo on floating point types not possible", line, column, true);
}

Type BinaryExpr::type() const
{
    const auto base = Type::combine(lhs->type(), rhs->type());
    return Type(false, base);
}

std::string PostfixExpr::name() const
{
    return "postfix expression";
}

std::string PostfixExpr::value() const
{
    return operation;
}

std::vector<Node*> PostfixExpr::children() const
{
    return {variable};
}

Literal* PostfixExpr::fold()
{
    return nullptr;
}

void PostfixExpr::check() const
{
    if (table->lookup_const(variable->name()))
    {
        throw ConstError("postfix expression", variable->name(), line, column);
    }
}

Type PostfixExpr::type() const
{
    return variable->type();
}

std::string PrefixExpr::name() const
{
    return "prefix expression";
}

std::string PrefixExpr::value() const
{
    return operation;
}

std::vector<Node*> PrefixExpr::children() const
{
    return {variable};
}

Literal* PrefixExpr::fold()
{
    return nullptr;
}

void PrefixExpr::check() const
{
    if (table->lookup_const(variable->name()))
    {
        throw ConstError("prefix expression", variable->name(), line, column);
    }
}

Type PrefixExpr::type() const
{
    return variable->type();
}

std::string UnaryExpr::name() const
{
    return "unary expression";
}

std::string UnaryExpr::value() const
{
    return operation;
}

std::vector<Node*> UnaryExpr::children() const
{
    return {operand};
}

Literal* UnaryExpr::fold()
{
    auto* new_operand = operand->fold();
    const auto lambda = [&](const auto& val)
    { return fold_unary(val, operation, table, line, column); };

    // TODO: deletus feetus, memory leakus
    if (new_operand) return std::visit(lambda, new_operand->literal);
    return nullptr;
}

void UnaryExpr::check() const
{
}

Type UnaryExpr::type() const
{
    return operand->type();
}

std::string CastExpr::name() const
{
    return "cast expression";
}

std::string CastExpr::value() const
{
    return '('+ cast.string()+')';
}

std::vector<Node*> CastExpr::children() const
{
    return {operand};
}

Literal* CastExpr::fold()
{
    auto* new_operand = operand->fold();
    const auto lambda = [&](const auto& val)
    { return fold_cast(val, cast.string(), table, line, column); };

    if (new_operand) return std::visit(lambda, new_operand->literal);
    else
        return nullptr;
}

void CastExpr::check() const
{
    if(cast.isPointerType())
    {
        std::cout << InternalError("casting to pointers is not yet supported", line, column);
    }
}

Type CastExpr::type() const
{
    return cast;
}

std::string Assignment::name() const
{
    return "assignment";
}

std::string Assignment::value() const
{
    return "";
}

std::vector<Node*> Assignment::children() const
{
    return {variable, expr};
}

Literal* Assignment::fold()
{
    assign_fold(expr);
    return nullptr;
}

void Assignment::check() const
{
    const auto rtype = expr->type();
    const auto ltype = variable->type();

    if(rtype.isPointerType())
    {
        if(ltype.isFloatingType())
            std::cout << ImpossibleConversion("assigning", rtype.string(), ltype.string(), line, column);
        else if(ltype.isIntegralType())
            std::cout << NarrowingConversion("assigning", rtype.string(), ltype.string(), line, column);
    }
    if(ltype.isPointerType())
    {
        if(rtype.isFloatingType())
            std::cout << ImpossibleConversion("assigning", rtype.string(), ltype.string(), line, column);
        else if(rtype.isIntegralType())
            std::cout << NarrowingConversion("assigning", rtype.string(), ltype.string(), line, column);
    }

    if (table->lookup_const(variable->name()))
    {
        std::cout << ConstError("assignment", variable->name(), line, column);
    }
}

Type Assignment::type() const
{
    return variable->type();
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
    if (expr) return {variable, expr};
    else
        return {variable};
}

Literal* Declaration::fold()
{
    if (not expr) return nullptr;

    if (auto* res = expr->fold())
    {
        if (variable->entry->second.type.isConst())
        {
            table->set_literal(variable->name(), res->literal);
        }
        expr = res;
    }
    return nullptr;
}

void Declaration::check() const
{
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
    return {expr};
}

Literal* PrintfStatement::fold()
{
    assign_fold(expr);
    return nullptr;
}

void PrintfStatement::check() const
{
}
} // namespace Ast