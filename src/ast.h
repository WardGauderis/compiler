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
    explicit Node(std::shared_ptr<SymbolTable> table, size_t column, size_t line)
        : table(std::move(table)), column(column), line(line)
    {
    }

    friend std::ofstream& operator<<(std::ofstream& stream, const std::unique_ptr<Node>& root);

    void complete(bool check, bool fold, bool output);

    [[nodiscard]] virtual std::string name() const = 0;

    [[nodiscard]] virtual std::string value() const = 0;

    [[nodiscard]] virtual std::vector<Node*> children() const = 0;

    [[nodiscard]] virtual std::string color() const = 0;

    virtual Literal* fold() = 0;

    virtual void check(std::ostream& error, std::ostream& warning) const = 0;

    //    virtual void llvm(std::ofstream& stream) = 0;

    size_t column;
    size_t line;

    std::shared_ptr<SymbolTable> table;
};

struct Statement : public Node
{
    explicit Statement(std::shared_ptr<SymbolTable> table, size_t column, size_t line)
        : Node(std::move(table), column, line)
    {
    }

    [[nodiscard]] std::string color() const override;
};

struct Expr : public Statement
{
    explicit Expr(std::shared_ptr<SymbolTable> table, size_t column, size_t line)
        : Statement(std::move(table), column, line)
    {
    }

    [[nodiscard]] std::string color() const override;
    [[nodiscard]] virtual Type type() const = 0;
};

struct Comment final : public Node
{
    explicit Comment(std::string comment, std::shared_ptr<SymbolTable> table, size_t column, size_t line)
        : Node(std::move(table), column, line), comment(std::move(comment))
    {
    }

    [[nodiscard]] std::string name() const final;
    [[nodiscard]] std::string value() const final;
    [[nodiscard]] std::vector<Node*> children() const final;
    [[nodiscard]] std::string color() const final;
    Literal* fold() final;
    void check(std::ostream& error, std::ostream& warning) const final;

    std::string comment;
};

struct Block final : public Node
{
    explicit Block(std::vector<Node*> nodes, std::shared_ptr<SymbolTable> table, size_t column, size_t line)
        :Node(std::move(table), column, line), nodes(std::move(nodes))
    {
    }

    [[nodiscard]] std::string name() const final;
    [[nodiscard]] std::string value() const final;
    [[nodiscard]] std::vector<Node*> children() const final;
    [[nodiscard]] std::string color() const final;
    Literal* fold() final;
    void check(std::ostream& error, std::ostream& warning) const final;

    std::vector<Node*> nodes;
};

struct Literal final : public Expr
{
    template<typename Type>
    explicit Literal(Type val, std::shared_ptr<SymbolTable> table, size_t column, size_t line)
        : Expr(std::move(table), column, line), literal(val)
    {
    }

    [[nodiscard]] std::string name() const final;
    [[nodiscard]] std::string value() const final;
    [[nodiscard]] std::vector<Node*> children() const final;
    Literal* fold() final;
    void check(std::ostream& error, std::ostream& warning) const final;
    [[nodiscard]] Type type() const final;

    TypeVariant literal;
};

struct Variable final : public Expr
{
    explicit Variable(SymbolTable::Entry entry, std::shared_ptr<SymbolTable> table, size_t column, size_t line)
        : Expr(std::move(table), column, line), entry(entry)
    {
    }

    [[nodiscard]] std::string name() const final;
    [[nodiscard]] std::string value() const final;
    [[nodiscard]] std::vector<Node*> children() const final;
    [[nodiscard]] std::string color() const final;
    Literal* fold() final;
    void check(std::ostream& error, std::ostream& warning) const final;
    [[nodiscard]] Type type() const final;

    SymbolTable::Entry entry;
};

struct BinaryExpr final : public Expr
{
    explicit BinaryExpr(std::string operation, Expr* lhs, Expr* rhs, std::shared_ptr<SymbolTable> table, size_t column, size_t line)
        : Expr(std::move(table), column, line), operation(std::move(operation)), lhs(lhs), rhs(rhs)
    {
    }

    [[nodiscard]] std::string name() const override;
    [[nodiscard]] std::string value() const override;
    [[nodiscard]] std::vector<Node*> children() const override;
    Literal* fold() final;
    void check(std::ostream& error, std::ostream& warning) const final;
    [[nodiscard]] Type type() const final;

    std::string operation;

    Expr* lhs;
    Expr* rhs;
};

struct PostfixExpr final : public Expr
{
    explicit PostfixExpr(std::string operation, Variable* variable, std::shared_ptr<SymbolTable> table, size_t column, size_t line)
        : Expr(std::move(table), column, line), operation(std::move(operation)), variable(variable)
    {
    }

    [[nodiscard]] std::string name() const final;
    [[nodiscard]] std::string value() const final;
    [[nodiscard]] std::vector<Node*> children() const final;
    Literal* fold() final;
    void check(std::ostream& error, std::ostream& warning) const final;
    [[nodiscard]] Type type() const final;

    std::string operation;
    Variable* variable;
};

struct PrefixExpr final : public Expr
{
    explicit PrefixExpr(std::string operation, Variable* variable, std::shared_ptr<SymbolTable> table, size_t column, size_t line)
        : Expr(std::move(table), column, line), operation(std::move(operation)), variable(variable)
    {
    }

    [[nodiscard]] std::string name() const final;
    [[nodiscard]] std::string value() const final;
    [[nodiscard]] std::vector<Node*> children() const final;
    Literal* fold() final;
    void check(std::ostream& error, std::ostream& warning) const final;
    [[nodiscard]] Type type() const final;

    std::string operation;
    Variable* variable;
};

struct UnaryExpr final : public Expr
{
    explicit UnaryExpr(std::string operation, Expr* operand, std::shared_ptr<SymbolTable> table, size_t column, size_t line)
        : Expr(std::move(table), column, line), operation(std::move(operation)), operand(operand)
    {
    }

    [[nodiscard]] std::string name() const final;
    [[nodiscard]] std::string value() const final;
    [[nodiscard]] std::vector<Node*> children() const final;
    Literal* fold() final;
    void check(std::ostream& error, std::ostream& warning) const final;
    [[nodiscard]] Type type() const final;

    std::string operation;
    Expr* operand;
};

struct CastExpr final : public Expr
{
    explicit CastExpr(Type cast, Expr* operand, std::shared_ptr<SymbolTable> table, size_t column, size_t line)
        : Expr(std::move(table), column, line), cast(cast), operand(operand)
    {
    }

    [[nodiscard]] std::string name() const final;
    [[nodiscard]] std::string value() const final;
    [[nodiscard]] std::vector<Node*> children() const final;
    Literal* fold() final;
    void check(std::ostream& error, std::ostream& warning) const final;
    [[nodiscard]] Type type() const final;

    Type cast;
    Expr* operand;
};

struct Assignment final : public Expr
{
    explicit Assignment(Variable* variable, Expr* expr, std::shared_ptr<SymbolTable> table, size_t column, size_t line)
        : Expr(std::move(table), column, line), variable(variable), expr(expr)
    {
    }

    [[nodiscard]] std::string name() const final;
    [[nodiscard]] std::string value() const final;
    [[nodiscard]] std::vector<Node*> children() const final;
    Literal* fold() final;
    void check(std::ostream& error, std::ostream& warning) const final;
    [[nodiscard]] Type type() const final;

    Variable* variable;
    Expr* expr;
};

struct Declaration final : public Statement
{
    explicit Declaration(Variable* variable, Expr* expr, std::shared_ptr<SymbolTable> table, size_t column, size_t line)
        : Statement(std::move(table), column, line), variable(variable), expr(expr)
    {
    }

    [[nodiscard]] std::string name() const final;
    [[nodiscard]] std::string value() const final;
    [[nodiscard]] std::vector<Node*> children() const final;
    Literal* fold() final;
    void check(std::ostream& error, std::ostream& warning) const final;

    Variable* variable;
    Expr* expr; // can be nullptr
};

struct PrintfStatement final : public Statement
{
    explicit PrintfStatement(Expr* expr, std::shared_ptr<SymbolTable> table, size_t column, size_t line)
        : Statement(std::move(table), column, line), expr(expr)
    {
    }

    [[nodiscard]] std::string name() const final;
    [[nodiscard]] std::string value() const final;
    [[nodiscard]] std::vector<Node*> children() const final;
    Literal* fold() final;
    void check(std::ostream& error, std::ostream& warning) const final;

    Expr* expr;
};

} // namespace Ast
