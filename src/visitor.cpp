//============================================================================
// @author      : Thomas Dooms & Ward Gauderis
// @date        : 3/10/20
// @copyright   : BA2 Informatica - Thomas Dooms & Ward Gauderis - University of Antwerp
//============================================================================

#include <memory>
#include <tree/ParseTree.h>

#include "CParser.h"
#include "ast.h"
#include "errors.h"
#include "folding.h"
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
        if (size == context->children.size())
        {
            res = func(context);
        }
    }

    Ret* result()
    {
        if (res == nullptr) throw SemanticError("could not find visitor for " + name);
        else return res;
    }

    Ret* res = nullptr;
    antlr4::tree::ParseTree* context;
    std::string name;
};
}

//============================================================================

Ast::Comment* visitComment(antlr4::tree::ParseTree* context)
{
    return new Ast::Comment(context->getText());
}

Ast::Literal* visitLiteral(antlr4::tree::ParseTree* context)
{
    auto* terminal = dynamic_cast<antlr4::tree::TerminalNode*>(context->children[0]);
    if(terminal == nullptr) throw WhoopsiePoopsieError("literal node is not a terminal in the cst");

    switch(terminal->getSymbol()->getType())
    {
        case CParser::FLOAT:
            return new Ast::Literal(std::stof(terminal->getText()));
        case CParser::INT:
            return new Ast::Literal(std::stoi(terminal->getText()));
        case CParser::CHAR:
            return new Ast::Literal(terminal->getText()[0]);
        default:
            throw WhoopsiePoopsieError("unknown literal type, probably not yet implemented");
    }
}
Ast::Expr* visitLiteralOrVariable(antlr4::tree::ParseTree* context)
{
    if(typeid(*context) == typeid(CParser::LiteralContext))
    {
        return visitLiteral(context);
    }
    else if(typeid(*context) == typeid(antlr4::tree::TerminalNodeImpl))
    {
        return new Ast::Variable(context->getText());
    }
    else throw WhoopsiePoopsieError(std::string("unknown basic expression type: ") + typeid(*context).name());
}

Ast::Expr* visitBasicExpr(antlr4::tree::ParseTree* context)
{
    VisitorHelper<Ast::Expr> visitor(context, "basic expression");
    visitor(1, [](auto* context) -> Ast::Expr*
    {
        return visitLiteralOrVariable(context->children[0]);
    });
    visitor(3, [](auto* context)
    {
        return visitExpr(context->children[1]);
    });
    return visitor.result();
}

Ast::Expr* visitPostfixExpr(antlr4::tree::ParseTree* context)
{
    VisitorHelper<Ast::Expr> visitor(context, "postfix expression");
    visitor(1, [](auto* context)
    {
        return visitBasicExpr(context->children[0]);
    });
    visitor(2, [](auto* context)
    {
        const auto lhs = new Ast::Variable(context->children[0]->getText());
        return new Ast::PostfixExpr(context->children[1]->getText(), lhs);
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
      return new Ast::PrefixExpr(context->children[0]->getText(), rhs);
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
    visitor(2, [](auto* context)
    {
      return new Ast::Declaration(context->children[0]->getText(),
                                  context->children[1]->getText(),
                                  nullptr);
    });
    visitor(4, [](auto* context)
    {
      return new Ast::Declaration(context->children[0]->getText(),
                                  context->children[1]->getText(),
                                  visitExpr(context->children[3]->children[0]));
    });
    return visitor.result();
}

Ast::Statement* visitPrintf(antlr4::tree::ParseTree* context)
{
    auto* expr = visitLiteralOrVariable(context->children[2]);
    return new Ast::PrintfStatement(expr);
}

Ast::Statement* visitStatement(antlr4::tree::ParseTree* context)
{
    const auto child = context->children[0];
    const auto hash = typeid(*child).hash_code();

    if(hash == typeid(CParser::ExprContext).hash_code())
    {
        return new Ast::ExprStatement(visitExpr(child));
    }
    else if(hash == typeid(CParser::DeclarationContext).hash_code())
    {
        return visitDeclaration(child);
    }
    else if(hash == typeid(CParser::PrintfContext).hash_code())
    {
        return visitPrintf(child);
    }
    else throw WhoopsiePoopsieError(std::string("unknown statement type: ") + typeid(*context).name() + ":\n\t" + context->getText());
}

std::unique_ptr<Ast::Node> visitBlock(antlr4::tree::ParseTree* context)
{
    std::vector<Ast::Node*> nodes;
    for(size_t i = 0; i < context->children.size() - 1; i++)
    {
        const auto& child = context->children[i];
        if(typeid(*child) == typeid(CParser::CommentContext))
        {
            nodes.emplace_back(visitComment(child));
        }
        else if(typeid(*child) == typeid(CParser::StatementContext))
        {
            nodes.emplace_back(visitStatement(child));
        }
        else throw WhoopsiePoopsieError(std::string("unknown node type: ") + typeid(*child).name());
    }
	return std::make_unique<Ast::Block>(nodes);
    return foldNodes(nodes);
}