//============================================================================
// @author      : Thomas Dooms & Ward Gauderis
// @date        : 3/10/20
// @copyright   : BA2 Informatica - Thomas Dooms & Ward Gauderis - University of Antwerp
//============================================================================

#include "ast.h"
#include <iostream>
#include <functional>
#include <fstream>

namespace Ast
{
std::ofstream& operator<<(std::ofstream& stream, const std::unique_ptr<Node>& root)
{
    stream << "digraph G\n";
    stream << "{\n";

    std::function<void(Node*)> recursion = [&](Node* node)
    {
        stream << '"' << node << "\"[label=\""
               << node->name() << "\\n"
               << node->value() << "\", shape=box, style=filled, color=\""
               << node->color() << "\"];\n";

        for (const auto child : node->children())
        {
            stream << '"' << node << "\" -> \"" << child << "\";\n";
            recursion(child);
        }
    };
    recursion(root.get());
    stream << "}\n" << std::flush;
    return stream;
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

std::string Block::name() const
{
    return "file";
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

std::string Literal::name() const
{
    return "literal";
}
std::string Literal::value() const
{
    return std::visit([&](const auto& val){ return std::to_string(val); }, literal);
}
std::vector<Node*> Literal::children() const
{
    return {};
}

std::string Variable::name() const
{
    return "variable";
}
std::string Variable::value() const
{
    return identifier;
}
std::vector<Node*> Variable::children() const
{
    return {};
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

std::string CastExpr::name() const
{
    return "cast expression";
}
std::string CastExpr::value() const
{
    return '(' + type + ')';
}
std::vector<Node*> CastExpr::children() const
{
    return {operand};
}

std::string Assignment::name() const
{
    return "assignment";
}
std::string Assignment::value() const
{
    return variable;
}
std::vector<Node*> Assignment::children() const
{
    return {expr};
}

std::string Declaration::name() const
{
    return "declaration";
}
std::string Declaration::value() const
{
    return type + ": " + identifier;
}
std::vector<Node*> Declaration::children() const
{
    if(expr) return {expr};
    else return {};
}

std::string ExprStatement::name() const
{
    return "expression statement";
}
std::string ExprStatement::value() const
{
    return "";
}
std::vector<Node*> ExprStatement::children() const
{
    return {expr};
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


}