//
// Created by ward on 3/6/20.
//

#pragma once

#include <array>
#include <memory>
#include <optional>
#include <variant>
#include <vector>
#include "errors.h"

namespace Ast
{

struct Node
{
    explicit Node() = default;

    friend std::ofstream& operator<<(std::ofstream& stream, const std::unique_ptr<Node>& root);

    [[nodiscard]] virtual std::string name() const = 0;
    [[nodiscard]] virtual std::string value() const = 0;
    [[nodiscard]] virtual std::vector<Node*> children() const = 0;
    [[nodiscard]] virtual std::string color() const = 0;
};

struct Expr : public Node
{
    explicit Expr() = default;
    [[nodiscard]] std::string color() const final;
};

struct Statement : public Node
{
    Statement() = default;
    [[nodiscard]] std::string color() const final;
};

struct Comment final : public Node
{
    explicit Comment(std::string comment)
        : comment(std::move(comment)) {}

    [[nodiscard]] std::string name() const final;
    [[nodiscard]] std::string value() const final;
    [[nodiscard]] std::vector<Node*> children() const final;
    [[nodiscard]] std::string color() const final;

    std::string comment;
};

struct Block final : public Node
{
    explicit Block(std::vector<Node*> nodes)
    : nodes(std::move(nodes)) {}

	std::vector<Node*> nodes;

    [[nodiscard]] std::string name() const final;
    [[nodiscard]] std::string value() const final;
    [[nodiscard]] std::vector<Node*> children() const final;
    [[nodiscard]] std::string color() const final;
};

struct Literal final : public Expr
{
    template<typename Type>
    explicit Literal(Type val) : literal(val) {}

    [[nodiscard]] std::string name() const final;
    [[nodiscard]] std::string value() const final;
    [[nodiscard]] std::vector<Node*> children() const final;

    std::variant<char, short, int, long, float, double> literal;
};

struct Variable final : public Expr
{
    explicit Variable(std::string identifier)
        : identifier(std::move(identifier)) {}

    [[nodiscard]] std::string name() const final;
    [[nodiscard]] std::string value() const final;
    [[nodiscard]] std::vector<Node*> children() const final;

    std::string identifier;
};

struct BinaryExpr final : public Expr
{
    explicit BinaryExpr(std::string operation, Expr* lhs, Expr* rhs)
    : operation(std::move(operation)), lhs(lhs), rhs(rhs) {}

    [[nodiscard]] std::string name() const override;
    [[nodiscard]] std::string value() const override;
    [[nodiscard]] std::vector<Node*> children() const override;

    std::string operation;

    Expr* lhs;
    Expr* rhs;
};

struct PostfixExpr final : public Expr
{
    explicit PostfixExpr(std::string operation, Variable* variable)
    : operation(std::move(operation)), variable(variable) {}

    [[nodiscard]] std::string name() const final;
    [[nodiscard]] std::string value() const final;
    [[nodiscard]] std::vector<Node*> children() const final;

    std::string operation;
    Variable* variable;
};

struct PrefixExpr final : public Expr
{
    explicit PrefixExpr(std::string operation, Variable* variable)
        : operation(std::move(operation)), variable(variable) {}

    [[nodiscard]] std::string name() const final;
    [[nodiscard]] std::string value() const final;
    [[nodiscard]] std::vector<Node*> children() const final;

    std::string operation;
    Variable* variable;
};

struct UnaryExpr final : public Expr
{
    explicit UnaryExpr(std::string operation, Expr* operand)
    : operation(std::move(operation)), operand(operand) {}

    [[nodiscard]] std::string name() const final;
    [[nodiscard]] std::string value() const final;
    [[nodiscard]] std::vector<Node*> children() const final;

    std::string operation;
    Expr* operand;
};

struct CastExpr final : public Expr
{
    explicit CastExpr(std::string type, Expr* operand)
        : type(std::move(type)), operand(operand) {}

    [[nodiscard]] std::string name() const final;
    [[nodiscard]] std::string value() const final;
    [[nodiscard]] std::vector<Node*> children() const final;

    std::string type;
    Expr* operand;
};

struct Assignment final : public Expr
{
    explicit Assignment(std::string variable, Expr* expr)
    : variable(std::move(variable)), expr(expr) {}

    [[nodiscard]] std::string name() const final;
    [[nodiscard]] std::string value() const final;
    [[nodiscard]] std::vector<Node*> children() const final;

    std::string variable;
    Expr* expr;
};

struct Declaration final : public Statement
{
    explicit Declaration(std::string type, std::string identifier, Expr* expr)
    : type(std::move(type)), identifier(std::move(identifier)), expr(expr) {}

    [[nodiscard]] std::string name() const final;
    [[nodiscard]] std::string value() const final;
    [[nodiscard]] std::vector<Node*> children() const final;

    std::string type;
    std::string identifier;

    Expr* expr;  // can be nullptr
};

struct ExprStatement final : public Statement
{
    explicit ExprStatement(Expr* expr) : expr(expr) {}

    [[nodiscard]] std::string name() const final;
    [[nodiscard]] std::string value() const final;
    [[nodiscard]] std::vector<Node*> children() const final;

    Expr* expr;
};

struct PrintfStatement final : public Statement
{
    explicit PrintfStatement() = default;

    [[nodiscard]] std::string name() const final;
    [[nodiscard]] std::string value() const final;
    [[nodiscard]] std::vector<Node*> children() const final;
};



}

// helpers for the ast traversal
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

template<typename Func>
void downcast_expr(Ast::Expr* node, const Func& func)
{
    if     (auto* res = dynamic_cast<Ast::BinaryExpr*   >(node)) func(res);
    else if(auto* res = dynamic_cast<Ast::UnaryExpr*    >(node)) func(res);
    else if(auto* res = dynamic_cast<Ast::Literal*      >(node)) func(res);
    else if(auto* res = dynamic_cast<Ast::Variable*     >(node)) func(res);
    else if(auto* res = dynamic_cast<Ast::Assignment*   >(node)) func(res);
    else throw SyntaxError("unknown expression type");
}

template<typename Func>
void downcast_statement(Ast::Statement* node, const Func& func)
{

    if(auto* res = dynamic_cast<Ast::Declaration*  >(node)) func(res);
    else throw SyntaxError("unknown statement type");
}

template<typename Func>
void downcast_node(Ast::Node* node, const Func& func)
{
    if     (auto* res = dynamic_cast<Ast::Comment*      >(node)) func(res);
    else if(auto* res = dynamic_cast<Ast::Block*        >(node)) func(res);
    else if(auto* res = dynamic_cast<Ast::Expr*         >(node)) downcast_expr(res, func);
    else if(auto* res = dynamic_cast<Ast::Statement*    >(node)) downcast_statement(res, func);
    else throw SemanticError("unknown node type");
}






