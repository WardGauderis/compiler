//
// Created by ward on 3/6/20.
//

#pragma once

#include <array>
#include <memory>
#include <optional>
#include <variant>
#include <vector>

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

struct Block final : public Node
{
    explicit Block(std::vector<Statement*> expressions)
    : expressions(std::move(expressions)) {}

	std::vector<Statement*> expressions;

    [[nodiscard]] std::string name() const final;
    [[nodiscard]] std::string value() const final;
    [[nodiscard]] std::vector<Node*> children() const final;
    [[nodiscard]] std::string color() const final;
};

struct BinaryExpr final : public Expr
{
    explicit BinaryExpr(std::string operation, Expr* lhs, Expr* rhs)
    : operation(std::move(operation)), lhs(lhs), rhs(rhs) {}

	std::string operation;

	Expr* lhs;
    Expr* rhs;

    [[nodiscard]] std::string name() const override;
    [[nodiscard]] std::string value() const override;
    [[nodiscard]] std::vector<Node*> children() const override;
};

struct UnaryExpr final : public Expr
{
    explicit UnaryExpr(std::string operation, Expr* operand)
    : operation(std::move(operation)), operand(operand) {}

	std::string operation;
	Expr* operand;

    [[nodiscard]] std::string name() const final;
    [[nodiscard]] std::string value() const final;
    [[nodiscard]] std::vector<Node*> children() const final;
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

struct Variable : public Expr
{
    explicit Variable(std::string name, bool isConst)
    : identifier(std::move(name)), isConst(isConst) {}

    [[nodiscard]] std::string name() const override;
    [[nodiscard]] std::string value() const override;
    [[nodiscard]] std::vector<Node*> children() const override;

    std::string identifier;
    bool isConst;
};

struct Pointer final : public Variable
{
    explicit Pointer(std::string name, bool isConst, Variable* pointee)
    : Variable(std::move(name), isConst), pointee(pointee) {}

    [[nodiscard]] std::string name() const final;
    [[nodiscard]] std::string value() const final;
    [[nodiscard]] std::vector<Node*> children() const final;

    Variable* pointee;
};

struct Assignment final : public Statement
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
    explicit Declaration(Variable* assignee, Expr* expr)
    : assignee(assignee), expr(expr) {}

    [[nodiscard]] std::string name() const final;
    [[nodiscard]] std::string value() const final;
    [[nodiscard]] std::vector<Node*> children() const final;

    Variable* assignee;
    Expr* expr;  // can be nullptr
};



}

// helpers for the ast traversal
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

template<typename Func>
void downcast_call(Ast::Node* node, const Func& func)
{
    if     (auto* res = dynamic_cast<Ast::Block*     >(node)) func(res);
    else if(auto* res = dynamic_cast<Ast::BinaryExpr*>(node)) func(res);
    else if(auto* res = dynamic_cast<Ast::UnaryExpr* >(node)) func(res);
    else if(auto* res = dynamic_cast<Ast::Literal*   >(node)) func(res);
    else if(auto* res = dynamic_cast<Ast::Assignment*>(node)) func(res);
    // we need to swap the order of pointer and variable, because dynamic cast
    else if(auto* res = dynamic_cast<Ast::Pointer*   >(node)) func(res);
    else if(auto* res = dynamic_cast<Ast::Variable*  >(node)) func(res);
}




