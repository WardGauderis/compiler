//============================================================================
// @name        : detail.h
// @author      : Thomas Dooms
// @date        : 3/10/20
// @version     :
// @copyright   : BA1 Informatica - Thomas Dooms - University of Antwerp
// @description :
//============================================================================

#pragma once

#include <memory>
#include <tree/ParseTree.h>

#include "CParser.h"
#include "ast.h"

namespace
{
    template<typename Type>
    bool is_type(antlr4::tree::ParseTree* context)
    {
        return typeid(Type) == typeid(*context);
    }

    bool is_binary_expression(antlr4::tree::ParseTree* context)
    {
        return is_type<CParser::OrExpressionContext>(context)
            or is_type<CParser::AndExpressionContext>(context)
            or is_type<CParser::EqualityExpressionContext>(context)
            or is_type<CParser::RelationalExpressionContext>(context)
            or is_type<CParser::AdditiveExpressionContext>(context)
            or is_type<CParser::MultiplicativeExpressionContext>(context);
    }

    bool is_unary_expression(antlr4::tree::ParseTree* context)
    {
        return is_type<CParser::UnaryExpressionContext>(context);
    }

    bool is_expression(antlr4::tree::ParseTree* context)
    {
        return is_type<CParser::ExpressionContext>(context);
    }

    bool is_file(antlr4::tree::ParseTree* context)
    {
        return is_type<CParser::FileContext>(context);
    }

    bool is_basic_expression(antlr4::tree::ParseTree* context)
    {
        return is_type<CParser::BasicExpressionContext>(context);
    }
}

Expr* visitExpr(antlr4::tree::ParseTree* context)
{
    if(is_expression(context))
    {
        visitExpr(context->children[0]);
    }
    else if(is_binary_expression(context))
    {
        if(context->children.size() == 1)
        {
            visitExpr(context->children[0]);
        }
        else if(context->children.size() == 3)
        {
            const auto lhs = visitExpr(context->children[0]);
            const auto rhs = visitExpr(context->children[2]);

            return new BinaryExpr(std::move(context->children[1]->getText()), lhs, rhs);
        }
        else throw std::logic_error("binary expression must have 1 or 3 children");
    }
    else if(is_unary_expression(context))
    {
        if(context->children.size() == 1)
        {
            visitExpr(context->children[0]);
        }
        else if(context->children.size() == 2)
        {
            const auto operand = visitExpr(context->children[1]);
            return new UnaryExpr(std::move(context->children[0]->getText()), operand);
        }
        else throw std::logic_error("unary expression must have 1 or 2 children");
    }
    else if(is_basic_expression(context))
    {
        if(context->children.size() == 1)
        {
            return new Int(context->children[0]->getText());
        }
        else if(context->children.size() == 3)
        {
            visitExpr(context->children[1]);
        }
        else throw std::logic_error("basic expression must have 1 or 3 children");
    }
    else throw std::logic_error(std::string("unknown type: ") + typeid(*context).name());
}

std::unique_ptr<AstNode> visitFile(antlr4::tree::ParseTree* context)
{
    if(is_file(context))
    {
        const auto size = context->children.size() - 1;
        std::vector<Expr*> expressions(size);

        for(size_t i = 0; i < size; i++)
        {
            expressions[i] = visitExpr(context->children[i]);
        }

        return std::make_unique<File>(expressions);
    }
    else throw std::logic_error("context does not have file");
}