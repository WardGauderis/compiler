//============================================================================
// @name        : ast.h
// @author      : Thomas Dooms
// @date        : 3/1/20
// @version     : 
// @copyright   : BA1 Informatica - Thomas Dooms - University of Antwerp
// @description : 
//============================================================================


#pragma once

#include <vector>
#include <memory>
#include <variant>

enum class ASTType
{
    variable,
    constant,
    expression,
    expression_sequence,
    binary_operation,
    unary_operation,
    assign,
};



struct ASTNode
{
    ASTType type;
    std::string data;

    std::vector<std::unique_ptr<ASTNode>> children;
};



