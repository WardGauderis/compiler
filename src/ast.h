//============================================================================
// @author      : Thomas Dooms & Ward Gauderis
// @date        : 3/10/20
// @copyright   : BA2 Informatica - Thomas Dooms & Ward Gauderis - University of Antwerp
//============================================================================
#pragma once

#include <array>
#include <iostream>
#include <memory>
#include <optional>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include "errors.h"
#include "table.h"

namespace Ast
{
// pre declare literal for virtual function
struct Literal;

struct Node
{
    explicit Node(std::shared_ptr<SymbolTable> table)
    : table(std::move(table)) {}

    friend std::ofstream& operator<<(std::ofstream& stream, const std::unique_ptr<Node>& root);

    [[nodiscard]] virtual std::string name() const = 0;

    [[nodiscard]] virtual std::string value() const = 0;

    [[nodiscard]] virtual std::vector<Node*> children() const = 0;

    [[nodiscard]] virtual std::string color() const = 0;

    [[nodiscard]] virtual Literal* fold() = 0;

    std::shared_ptr<SymbolTable> table;
};

struct Statement : public Node
{
    explicit Statement(std::shared_ptr<SymbolTable> table)
    : Node(std::move(table)) {}

    [[nodiscard]] std::string color() const override;
};

struct Expr : public Statement
{
    explicit Expr(std::shared_ptr<SymbolTable> table)
    : Statement(std::move(table)) {}

    [[nodiscard]] std::string color() const override;
};

struct Comment final : public Node
{
    explicit Comment(std::string comment, std::shared_ptr<SymbolTable> table)
    : Node(std::move(table)), comment(std::move(comment))
    {
    }

    [[nodiscard]] std::string name() const final;
    [[nodiscard]] std::string value() const final;
    [[nodiscard]] std::vector<Node*> children() const final;
    [[nodiscard]] std::string color() const final;
    Literal* fold() final;

    std::string comment;
};

struct Block final : public Node
{
    explicit Block(std::vector<Node*> nodes, std::shared_ptr<SymbolTable> table)
    : Node(std::move(table)), nodes(std::move(nodes))
    {
    }

    [[nodiscard]] std::string name() const final;
    [[nodiscard]] std::string value() const final;
    [[nodiscard]] std::vector<Node*> children() const final;
    [[nodiscard]] std::string color() const final;
    Literal* fold() final;

    std::vector<Node*> nodes;
};

struct Literal final : public Expr
{
    template <typename Type>
    explicit Literal(Type val, std::shared_ptr<SymbolTable> table)
    : Expr(std::move(table)), literal(val)
    {
    }

    [[nodiscard]] std::string name() const final;
    [[nodiscard]] std::string value() const final;
    [[nodiscard]] std::vector<Node*> children() const final;
    Literal* fold() final;

    base_type literal;
};

struct Variable final : public Expr
{
    explicit Variable(SymbolTable::Entry entry, std::shared_ptr<SymbolTable> table)
    : Expr(std::move(table)), entry(entry)
    {
    }

    [[nodiscard]] std::string name() const final;
    [[nodiscard]] std::string value() const final;
    [[nodiscard]] std::vector<Node*> children() const final;
    [[nodiscard]] std::string color() const final;
    Literal* fold() final;

    SymbolTable::Entry entry;
//    Literal* literal; // can be nullptr, represents the compile time value if one exists
};

struct BinaryExpr final : public Expr
{
    explicit BinaryExpr(std::string operation, Expr* lhs, Expr* rhs, std::shared_ptr<SymbolTable> table)
        : Expr(std::move(table)), operation(std::move(operation)), lhs(lhs), rhs(rhs)
    {
    }

    [[nodiscard]] std::string name() const override;
    [[nodiscard]] std::string value() const override;
    [[nodiscard]] std::vector<Node*> children() const override;
    Literal* fold() final;

    std::string operation;

    Expr* lhs;
    Expr* rhs;
};

struct PostfixExpr final : public Expr
{
    explicit PostfixExpr(std::string operation, Variable* variable, std::shared_ptr<SymbolTable> table)
        : Expr(std::move(table)), operation(std::move(operation)), variable(variable)
    {
    }

    [[nodiscard]] std::string name() const final;
    [[nodiscard]] std::string value() const final;
    [[nodiscard]] std::vector<Node*> children() const final;
    Literal* fold() final;

    std::string operation;
    Variable* variable;
};

struct PrefixExpr final : public Expr
{
    explicit PrefixExpr(std::string operation, Variable* variable, std::shared_ptr<SymbolTable> table)
        : Expr(std::move(table)), operation(std::move(operation)), variable(variable)
    {
    }

    [[nodiscard]] std::string name() const final;
    [[nodiscard]] std::string value() const final;
    [[nodiscard]] std::vector<Node*> children() const final;
    Literal* fold() final;

    std::string operation;
    Variable* variable;
};

struct UnaryExpr final : public Expr
{
    explicit UnaryExpr(std::string operation, Expr* operand, std::shared_ptr<SymbolTable> table)
        : Expr(std::move(table)), operation(std::move(operation)), operand(operand)
    {
    }

    [[nodiscard]] std::string name() const final;
    [[nodiscard]] std::string value() const final;
    [[nodiscard]] std::vector<Node*> children() const final;
    Literal* fold() final;

    std::string operation;
    Expr* operand;
};

struct CastExpr final : public Expr
{
    explicit CastExpr(Type* type, Expr* operand, std::shared_ptr<SymbolTable> table)
    : Expr(std::move(table)), type(type), operand(operand)
    {
    }

    [[nodiscard]] std::string name() const final;
    [[nodiscard]] std::string value() const final;
    [[nodiscard]] std::vector<Node*> children() const final;
    Literal* fold() final;

    Type* type;
    Expr* operand;
};

struct Assignment final : public Expr
{
    explicit Assignment(Variable* variable, Expr* expr, std::shared_ptr<SymbolTable> table)
    : Expr(std::move(table)), variable(variable), expr(expr)
    {
    }

    [[nodiscard]] std::string name() const final;
    [[nodiscard]] std::string value() const final;
    [[nodiscard]] std::vector<Node*> children() const final;
    Literal* fold() final;

    Variable* variable;
    Expr* expr;
};

struct Declaration final : public Statement
{
    explicit Declaration(Variable* variable, Expr* expr, std::shared_ptr<SymbolTable> table)
    : Statement(std::move(table)), variable(variable), expr(expr)
    {
    }

    [[nodiscard]] std::string name() const final;
    [[nodiscard]] std::string value() const final;
    [[nodiscard]] std::vector<Node*> children() const final;
    Literal* fold() final;

    Variable* variable;
    Expr* expr; // can be nullptr
};

struct PrintfStatement final : public Statement
{
    explicit PrintfStatement(Expr* expr, std::shared_ptr<SymbolTable> table)
    : Statement(std::move(table)), expr(expr)
    {
    }

    [[nodiscard]] std::string name() const final;
    [[nodiscard]] std::string value() const final;
    [[nodiscard]] std::vector<Node*> children() const final;
    Literal* fold() final;

    Expr* expr;
};

} // namespace Ast
