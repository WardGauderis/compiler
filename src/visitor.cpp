//============================================================================
// @name        : visitor.cpp
// @author      : Thomas Dooms
// @date        : 3/13/20
// @version     :
// @copyright   : BA1 Informatica - Thomas Dooms - University of Antwerp
// @description :
//============================================================================

#include <memory>
#include <tree/ParseTree.h>

#include "CParser.h"
#include "ast.h"
#include "errors.h"
#include "visitor.h"

namespace
{
template <typename Ret>
struct VisitorHelper
{
    VisitorHelper(antlr4::tree::ParseTree* context, std::string name)
        : context(context), name(std::move(name)) {}

    template <typename Func>
    void operator()(size_t size, const Func& func)
    {
        if (not res and size == context->children.size())
        {
            res = func(context);
        }
    }

    Ret* result()
    {
        if (not res) throw std::logic_error("could not find visitor for: " + name);
        else return res;
    }

    Ret* res;
    antlr4::tree::ParseTree* context;
    std::string name;
};
}

///////////////////////

Ast::Expr* visitConstant(antlr4::tree::ParseTree* context)
{
    return new Ast::Literal(std::stoi(context->children[0]->getText()));
}

Ast::Expr* visitBasicExpr(antlr4::tree::ParseTree* context)
{
    VisitorHelper<Ast::Expr> visitor(context, "basic expression");
    visitor(1, [](auto* context)
    {
        if(typeid(*context) == typeid(CParser::ConstantContext))
        {
            return visitConstant(context->children[0]);
        }
        else
        {
            Ast::Expr* res = new Ast::Variable(context->children[0]->getText());
            return res;
        }
    });
    visitor(3, [](auto* context)
    {
        return visitExpr(context->children[1]);
    });
}

Ast::Expr* visitPostfixExpr(antlr4::tree::ParseTree* context)
{
    VisitorHelper<Ast::Expr> visitor(context, "postfix expression");
    visitor(1, [](auto* context)
    {
        return visitBasicExpr(context->children[0]);
    });
    visitor(3, [](auto* context)
    {
        const auto lhs = new Ast::Variable(context->children[1]->getText());
        return new Ast::UnaryExpr(context->children[0]->getText(), lhs);
    });
    return visitor.result();
}

Ast::Expr* visitprefixExpr(antlr4::tree::ParseTree* context)
{
    VisitorHelper<Ast::Expr> visitor(context, "prefix expression");
    visitor(1, [](auto* context)
    {
      return visitPostfixExpr(context->children[0]);
    });
    visitor(2, [](auto* context)
    {
      const auto rhs = new Ast::Variable(context->children[1]->getText());
      return new Ast::UnaryExpr(context->children[0]->getText(), rhs);
    });
    return visitor.result();
}

Ast::Expr* visitUnaryExpr(antlr4::tree::ParseTree* context)
{
    VisitorHelper<Ast::Expr> visitor(context, "unary expression");
    visitor(1, [](auto* context)
    {
      return visitprefixExpr(context->children[0]);
    });
    visitor(2, [](auto* context)
    {
      const auto rhs = visitUnaryExpr(context->children[0]);
      return new Ast::UnaryExpr(context->children[1]->getText(), rhs);
    });
    visitor(4, [](auto* context)
    {
      const auto type = visitTypeName(context->children[0]);
      const auto rhs = visitUnaryExpr(context->children[3]);
      return new Ast::CastExpr(type, rhs);
    });
    return visitor.result();
}

Ast::Expr* visitMultiplicativeExpr(antlr4::tree::ParseTree* context)
{
    VisitorHelper<Ast::Expr> visitor(context, "multiplicative expression");
    visitor(1, [](auto* context)
    {
      return visitUnaryExpr(context->children[0]);
    });
    visitor(3, [](auto* context)
    {
      const auto lhs = visitMultiplicativeExpr(context->children[0]);
      const auto rhs = visitUnaryExpr(context->children[2]);
      return new Ast::BinaryExpr(context->children[1]->getText(), lhs, rhs);
    });
    return visitor.result();
}

Ast::Expr* visitAdditiveExpr(antlr4::tree::ParseTree* context)
{
    VisitorHelper<Ast::Expr> visitor(context, "additive expression");
    visitor(1, [](auto* context)
    {
      return visitMultiplicativeExpr(context->children[0]);
    });
    visitor(3, [](auto* context)
    {
      const auto lhs = visitAdditiveExpr(context->children[0]);
      const auto rhs = visitMultiplicativeExpr(context->children[2]);
      return new Ast::BinaryExpr(context->children[1]->getText(), lhs, rhs);
    });
    return visitor.result();
}

Ast::Expr* visitRelationalyExpr(antlr4::tree::ParseTree* context)
{
    VisitorHelper<Ast::Expr> visitor(context, "relational expression");
    visitor(1, [](auto* context)
    {
      return visitAdditiveExpr(context->children[0]);
    });
    visitor(3, [](auto* context)
    {
      const auto lhs = visitRelationalyExpr(context->children[0]);
      const auto rhs = visitAdditiveExpr(context->children[2]);
      return new Ast::BinaryExpr(context->children[1]->getText(), lhs, rhs);
    });
    return visitor.result();
}

Ast::Expr* visitEqualityExpr(antlr4::tree::ParseTree* context)
{
    VisitorHelper<Ast::Expr> visitor(context, "equality expression");
    visitor(1, [](auto* context)
    {
      return visitRelationalyExpr(context->children[0]);
    });
    visitor(3, [](auto* context)
    {
      const auto lhs = visitEqualityExpr(context->children[0]);
      const auto rhs = visitRelationalyExpr(context->children[2]);
      return new Ast::BinaryExpr(context->children[1]->getText(), lhs, rhs);
    });
    return visitor.result();
}

Ast::Expr* visitAndExpr(antlr4::tree::ParseTree* context)
{
    VisitorHelper<Ast::Expr> visitor(context, "and expression");
    visitor(1, [](auto* context)
    {
      return visitEqualityExpr(context->children[0]);
    });
    visitor(3, [](auto* context)
    {
      const auto lhs = visitAndExpr(context->children[0]);
      const auto rhs = visitEqualityExpr(context->children[2]);
      return new Ast::BinaryExpr("&&", lhs, rhs);
    });
    return visitor.result();
}

Ast::Expr* visitOrExpr(antlr4::tree::ParseTree* context)
{
    VisitorHelper<Ast::Expr> visitor(context, "or expression");
    visitor(1, [](auto* context)
    {
      return visitAndExpr(context->children[0]);
    });
    visitor(3, [](auto* context)
    {
      const auto lhs = visitOrExpr(context->children[0]);
      const auto rhs = visitAndExpr(context->children[2]);
      return new Ast::BinaryExpr("||", lhs, rhs);
    });
    return visitor.result();
}

Ast::Expr* visitAssignExpr(antlr4::tree::ParseTree* context)
{
    VisitorHelper<Ast::Expr> visitor(context, "assign expression");
    visitor(1, [](auto* context)
    {
      return visitOrExpr(context->children[0]);
    });
    visitor(3, [](auto* context)
    {
      const auto identifier = context->children[0]->getText();
      const auto rhs = visitAssignExpr(context->children[2]);
      return new Ast::Assignment(identifier, rhs);
    });
    return visitor.result();
}

Ast::Expr* visitExpr(antlr4::tree::ParseTree* context)
{
    VisitorHelper<Ast::Expr> visitor(context, "expression");
    visitor(1, [](auto* context)
    {
      return visitAssignExpr(context->children[0]);
    });
    return visitor.result();
}

std::string visitTypeName(antlr4::tree::ParseTree* context)
{
    return context->getText();
}

std::string visitPointerType(antlr4::tree::ParseTree* context)
{
    return context->getText();
}

Ast::Expr* visitInitializer(antlr4::tree::ParseTree* context)
{
    return visitExpr(context->children[0]);
}

Ast::Statement* visitDeclaration(antlr4::tree::ParseTree* context)
{
    VisitorHelper<Ast::Statement> visitor(context, "statement");
    visitor(3, [](auto* context)
    {
      return new Ast::Declaration(context->children[0]->getText(),
                                  context->children[1]->getText(),
                                  nullptr);
    });
    visitor(5, [](auto* context)
    {
      return new Ast::Declaration(context->children[0]->getText(),
                                  context->children[1]->getText(),
                                  visitExpr(context->children[3]->children[0]));
    });
    return visitor.result();
}

Ast::Statement* visitStatement(antlr4::tree::ParseTree* context)
{
    std::cout << context->children.size() << '\n';
    const auto child = context->children[0];
    const auto hash = typeid(*child).hash_code();

    if(hash == typeid(CParser::AssignExprContext).hash_code())
    {
        return new Ast::UnusedExpr(visitExpr(child));
    }
    else if(hash == typeid(CParser::DeclarationContext).hash_code())
    {
        return visitDeclaration(child);
    }
    else throw std::logic_error(std::string("unknown statement type: ") + typeid(*context).name());
}

std::unique_ptr<Ast::Node> visitBlock(antlr4::tree::ParseTree* context)
{
    const auto size = context->children.size() - 1;
    std::vector<Ast::Statement*> exprs(size);

    for(size_t i = 0; i < size; i++)
    {
        exprs[i] = visitStatement(context->children[i]);
    }
    auto result = std::make_unique<Ast::Block>(exprs);
    return result;
}