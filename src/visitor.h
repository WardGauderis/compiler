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
#include "folding.h"

namespace
{
    template<typename Type>
    bool is_type(antlr4::tree::ParseTree* context)
    {
        return typeid(Type) == typeid(*context);
    }

    bool is_binary_expr(antlr4::tree::ParseTree* context)
    {
        return is_type<CParser::OrExprContext>(context)
            or is_type<CParser::AndExprContext>(context)
            or is_type<CParser::EqualityExprContext>(context)
            or is_type<CParser::RelationalExprContext>(context)
            or is_type<CParser::AdditiveExprContext>(context)
            or is_type<CParser::MultiplicativeExprContext>(context);
    }

    bool is_prefix_expr(antlr4::tree::ParseTree* context)
    {
        return is_type<CParser::PrefixExprContext>(context);
    }

    bool is_postfix_expr(antlr4::tree::ParseTree* context)
    {
        return is_type<CParser::PostfixExprContext>(context);
    }

    bool is_expr(antlr4::tree::ParseTree* context)
    {
        return is_type<CParser::ExprContext>(context);
    }

    bool is_file(antlr4::tree::ParseTree* context)
    {
        return is_type<CParser::FileContext>(context);
    }

    bool is_basic_expr(antlr4::tree::ParseTree* context)
    {
        return is_type<CParser::BasicExprContext>(context);
    }

    bool is_assign_expr(antlr4::tree::ParseTree* context)
    {
        return is_type<CParser::AssignExprContext>(context);
    }

    bool is_declaration(antlr4::tree::ParseTree* context)
    {
        return is_type<CParser::DeclarationContext>(context);
    }

    bool is_initializer(antlr4::tree::ParseTree* context)
    {
        return is_type<CParser::InitizalizerContext>(context);
    }

    bool is_typename(antlr4::tree::ParseTree* context)
    {
        return is_type<CParser::TypeNameContext>(context);
    }

    bool is_pointer_type(antlr4::tree::ParseTree* context)
    {
        return is_type<CParser::PointerTypeContext>(context);
    }
}

Ast::Expr* visitExpr(antlr4::tree::ParseTree* context)
{
    if(is_expr(context))
    {
        visitExpr(context->children[0]);
    }
    else if(is_binary_expr(context))
    {
        if(context->children.size() == 1)
        {
            visitExpr(context->children[0]);
        }
        else if(context->children.size() == 3)
        {
            const auto lhs = visitExpr(context->children[0]);
            const auto rhs = visitExpr(context->children[2]);

            return new Ast::BinaryExpr(std::move(context->children[1]->getText()), lhs, rhs);
        }
        else throw std::logic_error("binary expr must have 1 or 3 children");
    }
    else if(is_prefix_expr(context))
    {
        if(context->children.size() == 1)
        {
            visitExpr(context->children[0]);
        }
        else if(context->children.size() == 2)
        {
            const auto operand = visitExpr(context->children[1]);
            return new Ast::UnaryExpr(std::move(context->children[0]->getText()), operand);
        }
        else throw std::logic_error("unary expr must have 1 or 2 children");
    }
    else if(is_basic_expr(context))
    {
        if(context->children.size() == 1)
        {
            const auto num = std::stoi(context->children[0]->getText());
            return new Ast::Literal(num);
        }
        else if(context->children.size() == 3)
        {
            visitExpr(context->children[1]);
        }
        else throw std::logic_error("basic expr must have 1 or 3 children");
    }
    else throw std::logic_error(std::string("unknown type: ") + typeid(*context).name());
}

Ast::Statement* visitStatement(antlr4::tree::ParseTree* context)
{
    if(is_assign_expr(context))
    {
        if(context->children.size() == 1)
        {
            visitExpr(context->children[0]);
        }
        else if(context->children.size() == 3)
        {
            const auto expr = visitExpr(context->children[2]);
            return new Ast::Assignment(context->children[0]->getText(), expr);
        }
        else throw std::logic_error("assign expr must have 1 or 3 children");
    }
    else if(is_declaration(context))
    {
        if(context->children.size() == 3)
        {
            //TODO figure out good system for typenames and such
        }
        else if(context->children.size() == 5)
        {

        }
        else throw std::logic_error("declaration must have 3 or 5 children");
    }
}

std::unique_ptr<Ast::Node> visitFile(antlr4::tree::ParseTree* context)
{
    if(is_file(context))
    {
        const auto size = context->children.size() - 1;
        std::vector<Ast::Statement*> exprs(size);

        for(size_t i = 0; i < size; i++)
        {
            exprs[i] = visitStatement(context->children[i]);
        }
        auto result = std::make_unique<Ast::Block>(exprs);
        foldFile(result);
        return result;
    }
    else throw std::logic_error("context does not have file");
}