//============================================================================
// @author      : Thomas Dooms
// @date        : 3/21/20
// @copyright   : BA2 Informatica - Thomas Dooms - University of Antwerp
//============================================================================

#pragma once
#include "node.h"


namespace Ast
{

struct Expr : public Statement
{
    explicit Expr(std::shared_ptr<SymbolTable> table, size_t line, size_t column)
    : Statement(std::move(table), line, column)
    {
    }

    [[nodiscard]] std::string   color() const override;
    [[nodiscard]] virtual Type* type() const     = 0;
    [[nodiscard]] virtual bool  constant() const = 0;
};

struct Literal final : public Expr
{
    template <typename Variant>
    explicit Literal(Variant val, std::shared_ptr<SymbolTable> table, size_t line, size_t column)
    : Expr(std::move(table), line, column), literal(std::move(val))
    {
    }

    [[nodiscard]] std::string name() const final;
    [[nodiscard]] std::string value() const final;
    [[nodiscard]] Literal*    fold() final;
    [[nodiscard]] Type*       type() const final;
    [[nodiscard]] bool        constant() const final;
    void                      visit(IRVisitor& visitor) final;

    TypeVariant literal;
};

struct StringLiteral final : public Expr
{
    explicit StringLiteral(std::string val, std::shared_ptr<SymbolTable> table, size_t line, size_t column)
    : Expr(std::move(table), line, column), val(std::move(val))
    {
    }

    [[nodiscard]] std::string name() const final;
    [[nodiscard]] std::string value() const final;
    [[nodiscard]] Node*    fold() final;
    [[nodiscard]] Type*       type() const final;
    [[nodiscard]] bool        constant() const final;
    void                      visit(IRVisitor& visitor) final;

    std::string val;
};

struct Variable final : public Expr
{
    explicit Variable(std::string identifier, std::shared_ptr<SymbolTable> table, size_t line, size_t column)
    : Expr(std::move(table), line, column), identifier(std::move(identifier))
    {
    }

    [[nodiscard]] std::string name() const final;
    [[nodiscard]] std::string value() const final;
    [[nodiscard]] std::string color() const final;
    [[nodiscard]] Node*       fold() final;
    [[nodiscard]] bool        constant() const final;
    [[nodiscard]] bool        check() const final;

    [[nodiscard]] Type* type() const final;
    void                visit(IRVisitor& visitor) final;

    std::string identifier;
};

struct BinaryExpr final : public Expr
{
    explicit BinaryExpr(const std::string& operation, Expr* lhs, Expr* rhs, std::shared_ptr<SymbolTable> table, size_t line, size_t column)
    : Expr(std::move(table), line, column), operation(operation), lhs(lhs), rhs(rhs)
    {
    }

    [[nodiscard]] std::string        name() const override;
    [[nodiscard]] std::string        value() const override;
    [[nodiscard]] std::vector<Node*> children() const override;
    [[nodiscard]] Node*              fold() final;
    [[nodiscard]] bool               check() const final;
    [[nodiscard]] Type*              type() const final;
    [[nodiscard]] bool               constant() const final;
    void                             visit(IRVisitor& visitor) final;

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

    [[nodiscard]] std::string        name() const final;
    [[nodiscard]] std::string        value() const final;
    [[nodiscard]] std::vector<Node*> children() const final;
    [[nodiscard]] Node*              fold() final;
    [[nodiscard]] bool               check() const final;
    [[nodiscard]] Type*              type() const final;
    [[nodiscard]] bool               constant() const final;
    void                             visit(IRVisitor& visitor) final;

    PostfixOperation operation;
    Expr*            operand;
};

struct PrefixExpr final : public Expr
{
    explicit PrefixExpr(const std::string& operation, Expr* operand, std::shared_ptr<SymbolTable> table, size_t line, size_t column)
    : Expr(std::move(table), line, column), operation(operation), operand(operand)
    {
    }

    [[nodiscard]] std::string        name() const final;
    [[nodiscard]] std::string        value() const final;
    [[nodiscard]] std::vector<Node*> children() const final;
    [[nodiscard]] Node*              fold() final;
    [[nodiscard]] bool               check() const final;
    [[nodiscard]] Type*              type() const final;
    [[nodiscard]] bool               constant() const final;
    void                             visit(IRVisitor& visitor) final;

    PrefixOperation operation;
    Expr*           operand;
};

struct CastExpr final : public Expr
{
    explicit CastExpr(Type* cast, Expr* operand, std::shared_ptr<SymbolTable> table, size_t line, size_t column)
    : Expr(std::move(table), line, column), cast(std::move(cast)), operand(operand)
    {
    }

    [[nodiscard]] std::string        name() const final;
    [[nodiscard]] std::string        value() const final;
    [[nodiscard]] std::vector<Node*> children() const final;
    [[nodiscard]] Node*              fold() final;
    [[nodiscard]] bool               check() const final;
    [[nodiscard]] Type*              type() const final;
    [[nodiscard]] bool               constant() const final;
    void                             visit(IRVisitor& visitor) final;

    Type* cast;
    Expr* operand;
};

struct Assignment final : public Expr
{
    explicit Assignment(Expr* lhs, Expr* rhs, std::shared_ptr<SymbolTable> table, size_t line, size_t column)
    : Expr(std::move(table), line, column), lhs(lhs), rhs(rhs)
    {
    }

    [[nodiscard]] std::string        name() const final;
    [[nodiscard]] std::vector<Node*> children() const final;
    [[nodiscard]] Node*              fold() final;
    [[nodiscard]] bool               check() const final;
    [[nodiscard]] Type*              type() const final;
    [[nodiscard]] bool               constant() const final;
    void                             visit(IRVisitor& visitor) final;

    Expr* lhs;
    Expr* rhs;
};

struct FunctionCall final : public Expr
{
    FunctionCall(std::vector<Expr*> arguments, std::string identifier, std::shared_ptr<SymbolTable> table, size_t line, size_t column)
    : Expr(std::move(table), line, column), arguments(std::move(arguments)),
      identifier(std::move(identifier))
    {
    }

    [[nodiscard]] std::string        name() const final;
    [[nodiscard]] std::string        value() const final;
    [[nodiscard]] std::vector<Node*> children() const final;
    [[nodiscard]] Node*              fold() final;
    [[nodiscard]] bool               check() const final;
    [[nodiscard]] Type*              type() const final;
    [[nodiscard]] bool               constant() const final;
    void                             visit(IRVisitor& visitor) final;

    std::vector<Expr*> arguments;
    std::string        identifier;
};

struct SubscriptExpr final : public Expr
{
    SubscriptExpr(Expr* lhs, Expr* rhs, std::shared_ptr<SymbolTable> table, size_t line, size_t column)
    : Expr(std::move(table), line, column), lhs(lhs), rhs(rhs)
    {
    }

    [[nodiscard]] std::string        name() const final;
    [[nodiscard]] std::string        value() const final;
    [[nodiscard]] std::vector<Node*> children() const final;
    [[nodiscard]] Node*              fold() final;
    [[nodiscard]] bool               check() const final;
    [[nodiscard]] Type*              type() const final;
    [[nodiscard]] bool               constant() const final;
    void                             visit(IRVisitor& visitor) final;

    Expr* lhs;
    Expr* rhs;
};

} // namespace Ast
