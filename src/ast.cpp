//
// Created by ward on 3/6/20.
//

#include "ast.h"
#include <iostream>
#include <functional>
#include <fstream>


std::ofstream& operator<<(std::ofstream& stream, const std::unique_ptr<AstNode>& root)
{
    std::function<void(AstNode*)> recursion = [&](AstNode* node)
    {
        stream << '"' << node << "\"[label=\"" << node->name() << "\\n" << node->value() << "\"];\n";
        for(const auto child : node->children())
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
std::vector<AstNode*> File::children() const
{
    std::vector<AstNode*> result(expressions.size());
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
std::vector<AstNode*> BinaryExpr::children() const
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
std::vector<AstNode*> UnaryExpr::children() const
{
    return {operand};
}

std::string Int::name() const
{
    return "literal";
}
std::string Int::value() const
{
    return std::to_string(val);
}
std::vector<AstNode*> Int::children() const
{
    return {};
}