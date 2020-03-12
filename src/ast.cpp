//
// Created by ward on 3/6/20.
//

#include "ast.h"
#include <iostream>
#include <functional>
#include <fstream>

namespace Ast
{
std::ofstream& operator<<(std::ofstream& stream, const std::unique_ptr<Node>& root)
{
    std::function<void(Node*)> recursion = [&](Node* node) {
        stream << '"' << node << "\"[label=\"" << node->name() << "\\n"
               << node->value() << "\"];\n";
        for (const auto child : node->children())
        {
            stream << '"' << node << "\" -> \"" << child << "\";\n";
            recursion(child);
        }
    };
    recursion(root.get());
    return stream;
}

std::string File::name() const
{
    return "file";
}
std::string File::value() const
{
    return "";
}
std::vector<Node*> File::children() const
{
    std::vector<Node*> result(expressions.size());
    std::copy(expressions.begin(), expressions.end(), result.begin());
    return result;
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

}