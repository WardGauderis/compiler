//============================================================================
// @author      : Thomas Dooms & Ward Gauderis
// @date        : 3/10/20
// @copyright   : BA2 Informatica - Thomas Dooms & Ward Gauderis - University of Antwerp
//============================================================================
#pragma once

#include <llvm/IR/Value.h>

#include <array>
#include <iostream>
#include <memory>
#include <optional>
#include <unordered_map>
#include <filesystem>
#include <utility>
#include <variant>
#include <vector>

#include "errors.h"
#include "table.h"

namespace Ast
{
// pre declare literal for virtual function
struct Literal;
struct Value;

struct Node
{
    explicit Node(std::shared_ptr<SymbolTable> table, size_t line, size_t column)
        : table(std::move(table)), column(column), line(line)
    {
    }

    friend std::ofstream& operator<<(std::ofstream& stream, const std::unique_ptr<Node>& root);

    void complete(bool check, bool fold, bool output);

    [[nodiscard]] virtual std::string name() const = 0;

    [[nodiscard]] virtual std::string value() const = 0;

    [[nodiscard]] virtual std::vector<Node*> children() const = 0;

    [[nodiscard]] virtual std::string color() const = 0;

    [[nodiscard]] virtual Literal* fold() = 0;

    [[nodiscard]] virtual bool check() const = 0;

    virtual llvm::Value* codegen() const = 0;

    size_t column;
    size_t line;

    std::shared_ptr<SymbolTable> table;
};

struct Statement : public Node
{
    explicit Statement(std::shared_ptr<SymbolTable> table, size_t line, size_t column)
        : Node(std::move(table), line, column)
    {
    }

    [[nodiscard]] std::string color() const override;
};

struct Expr : public Statement
{
    explicit Expr(std::shared_ptr<SymbolTable> table, size_t line, size_t column)
        : Statement(std::move(table), line, column)
    {
    }

    [[nodiscard]] std::string color() const override;
    [[nodiscard]] virtual Type type() const = 0;
};

struct Comment final : public Node
{
    explicit Comment(std::string comment, std::shared_ptr<SymbolTable> table, size_t line, size_t column)
        : Node(std::move(table), line, column), comment(std::move(comment))
    {
    }

    [[nodiscard]] std::string name() const final;
    [[nodiscard]] std::string value() const final;
    [[nodiscard]] std::vector<Node*> children() const final;
    [[nodiscard]] std::string color() const final;
    Literal* fold() final;
    bool check() const final;
    [[nodiscard]] llvm::Value* codegen() const final;

    std::string comment;
};

struct Block final : public Node
{
    explicit Block(std::vector<Node*> nodes, std::shared_ptr<SymbolTable> table, size_t line, size_t column)
        : Node(std::move(table), line, column), nodes(std::move(nodes))
    {
    }

    [[nodiscard]] std::string name() const final;
    [[nodiscard]] std::string value() const final;
    [[nodiscard]] std::vector<Node*> children() const final;
    [[nodiscard]] std::string color() const final;
    Literal* fold() final;
    bool check() const final;
    [[nodiscard]] llvm::Value* codegen() const final;

    std::vector<Node*> nodes;
};

struct Literal final : public Expr
{
    template<typename Type>
    explicit Literal(Type val, std::shared_ptr<SymbolTable> table, size_t line, size_t column)
        : Expr(std::move(table), line, column), literal(val)
    {
    }

    [[nodiscard]] std::string name() const final;
    [[nodiscard]] std::string value() const final;
    [[nodiscard]] std::vector<Node*> children() const final;
    Literal* fold() final;
    bool check() const final;
    [[nodiscard]] Type type() const final;
    [[nodiscard]] llvm::Value* codegen() const final;

    TypeVariant literal;
};

struct Variable final : public Expr
{
    explicit Variable(std::string identifier, std::shared_ptr<SymbolTable> table, size_t line, size_t column)
        : Expr(std::move(table), line, column), identifier(std::move(identifier))
    {
    }

    [[nodiscard]] std::string name() const final;
    [[nodiscard]] std::string value() const final;
    [[nodiscard]] std::vector<Node*> children() const final;
    [[nodiscard]] std::string color() const final;
    Literal* fold() final;
    bool check() const final;
    [[nodiscard]] Type type() const final;
    [[nodiscard]] llvm::Value* codegen() const final;

    [[nodiscard]] SymbolTable::Entry getEntry() const;
    [[nodiscard]] bool isConst() const;
    [[nodiscard]] bool declare(Type type) const;

    std::string identifier;
};

struct BinaryExpr final : public Expr
{
    explicit BinaryExpr(const std::string& operation, Expr* lhs, Expr* rhs, std::shared_ptr<SymbolTable> table, size_t line, size_t column)
        : Expr(std::move(table), line, column), operation(operation), lhs(lhs), rhs(rhs)
    {
    }

    [[nodiscard]] std::string name() const override;
    [[nodiscard]] std::string value() const override;
    [[nodiscard]] std::vector<Node*> children() const override;
    Literal* fold() final;
    bool check() const final;
    [[nodiscard]] Type type() const final;
    [[nodiscard]] llvm::Value* codegen() const final;

    BinaryOperation operation;

    Expr* lhs;
    Expr* rhs;
};

struct PostfixExpr final : public Expr
{
    explicit PostfixExpr(const std::string& operation, Expr* operand, std::shared_ptr<SymbolTable> table, size_t line, size_t column)
        : Expr(std::move(table), line, column), operation(operation), operand(operand)
    {
    }

    [[nodiscard]] std::string name() const final;
    [[nodiscard]] std::string value() const final;
    [[nodiscard]] std::vector<Node*> children() const final;
    Literal* fold() final;
    bool check() const final;
    [[nodiscard]] Type type() const final;
    [[nodiscard]] llvm::Value* codegen() const final;

    PostfixOperation operation;
    Expr* operand;
};

struct PrefixExpr final : public Expr
{
    explicit PrefixExpr(const std::string& operation, Expr* operand, std::shared_ptr<SymbolTable> table, size_t line, size_t column)
        : Expr(std::move(table), line, column), operation(operation), operand(operand)
    {
    }

    [[nodiscard]] std::string name() const final;
    [[nodiscard]] std::string value() const final;
    [[nodiscard]] std::vector<Node*> children() const final;
    Literal* fold() final;
    bool check() const final;
    [[nodiscard]] Type type() const final;
    [[nodiscard]] llvm::Value* codegen() const final;

    PrefixOperation operation;
    Expr* operand;
};

struct CastExpr final : public Expr
{
    explicit CastExpr(Type cast, Expr* operand, std::shared_ptr<SymbolTable> table, size_t line, size_t column)
        : Expr(std::move(table), line, column), cast(cast), operand(operand)
    {
    }

    [[nodiscard]] std::string name() const final;
    [[nodiscard]] std::string value() const final;
    [[nodiscard]] std::vector<Node*> children() const final;
    Literal* fold() final;
    bool check() const final;
    [[nodiscard]] Type type() const final;
    [[nodiscard]] llvm::Value* codegen() const final;

    Type cast;
    Expr* operand;
};

struct Assignment final : public Expr
{
    explicit Assignment(Variable* variable, Expr* expr, std::shared_ptr<SymbolTable> table, size_t line, size_t column)
        : Expr(std::move(table), line, column), variable(variable), expr(expr)
    {
    }

    [[nodiscard]] std::string name() const final;
    [[nodiscard]] std::string value() const final;
    [[nodiscard]] std::vector<Node*> children() const final;
    Literal* fold() final;
    bool check() const final;
    [[nodiscard]] Type type() const final;
    [[nodiscard]] llvm::Value* codegen() const final;

    Variable* variable;
    Expr* expr;
};

struct Declaration final : public Statement
{
    explicit Declaration(Type vartype, Variable* variable, Expr* expr, std::shared_ptr<SymbolTable> table, size_t line, size_t column)
        : Statement(std::move(table), line, column), vartype(vartype), variable(variable), expr(expr)
    {
    }

    [[nodiscard]] std::string name() const final;
    [[nodiscard]] std::string value() const final;
    [[nodiscard]] std::vector<Node*> children() const final;
    Literal* fold() final;
    bool check() const final;
    [[nodiscard]] llvm::Value* codegen() const final;

    Type vartype;
    Variable* variable;
    Expr* expr; // can be nullptr
};

struct PrintfStatement final : public Statement
{
    explicit PrintfStatement(Expr* expr, std::shared_ptr<SymbolTable> table, size_t line, size_t column)
        : Statement(std::move(table), line, column), expr(expr)
    {
    }

    [[nodiscard]] std::string name() const final;
    [[nodiscard]] std::string value() const final;
    [[nodiscard]] std::vector<Node*> children() const final;
    Literal* fold() final;
    bool check() const final;
    [[nodiscard]] llvm::Value* codegen() const final;

    Expr* expr;
};

	void ast2ir(const std::unique_ptr<Ast::Node>& root, const std::filesystem::path& path);

} // namespace Ast
