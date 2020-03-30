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

    [[nodiscard]] std::string  color() const override;
    [[nodiscard]] virtual Type type() const = 0;
    [[nodiscard]] virtual bool constant() const = 0;
};

struct Literal final : public Expr
{
    template <typename Type>
    explicit Literal(Type val, std::shared_ptr<SymbolTable> table, size_t line, size_t column)
    : Expr(std::move(table), line, column), literal(val)
    {
    }

  [[nodiscard]] std::string name() const final;
  [[nodiscard]] std::string value() const final;
  [[nodiscard]] std::vector<Node*> children() const final;
  [[nodiscard]] Literal* fold() final;
  [[nodiscard]] Type type() const final;
  [[nodiscard]] bool constant() const final;
  void visit(IRVisitor& visitor) final;

  TypeVariant literal;
};

struct Variable final : public Expr
{
    explicit Variable(std::string identifier, std::shared_ptr<SymbolTable> table, size_t line, size_t column)
    : Expr(std::move(table), line, column), identifier(std::move(identifier))
    {
    }

    [[nodiscard]] std::string        name() const final;
    [[nodiscard]] std::string        value() const final;
    [[nodiscard]] std::vector<Node*> children() const final;
    [[nodiscard]] std::string        color() const final;
    [[nodiscard]] Literal*           fold() final;
    [[nodiscard]] bool constant() const final;
    [[nodiscard]] bool               check() const final;

  [[nodiscard]] Type type() const final;
  void visit(IRVisitor& visitor) final;

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
  [[nodiscard]] Literal* fold() final;
  [[nodiscard]] bool check() const final;
  [[nodiscard]] Type type() const final;
    [[nodiscard]] bool constant() const final;
  void visit(IRVisitor& visitor) final;

    BinaryOperation operation;

    Expr* lhs;
    Expr* rhs;
};

struct PostfixExpr final : public Expr
{
    explicit PostfixExpr(const std::string&           operation,
                         Variable*                    variable,
                         std::shared_ptr<SymbolTable> table,
                         size_t                       line,
                         size_t                       column)
    : Expr(std::move(table), line, column), operation(operation), variable(variable)
    {
    }

  [[nodiscard]] std::string name() const final;
  [[nodiscard]] std::string value() const final;
  [[nodiscard]] std::vector<Node*> children() const final;
  [[nodiscard]] Literal* fold() final;
  [[nodiscard]] bool check() const final;
  [[nodiscard]] Type type() const final;
    [[nodiscard]] bool constant() const final;
  void visit(IRVisitor& visitor) final;

    PostfixOperation operation;
    Variable*        variable;
};

struct PrefixExpr final : public Expr
{
    explicit PrefixExpr(const std::string& operation, Variable* operand, std::shared_ptr<SymbolTable> table, size_t line, size_t column)
    : Expr(std::move(table), line, column), operation(operation), operand(operand)
    {
    }

    explicit PrefixExpr(const std::string& operation, Expr* operand, std::shared_ptr<SymbolTable> table, size_t line, size_t column)
    : Expr(std::move(table), line, column), operation(operation), operand(operand)
    {
    }

  [[nodiscard]] std::string name() const final;
  [[nodiscard]] std::string value() const final;
  [[nodiscard]] std::vector<Node*> children() const final;
  [[nodiscard]] Literal* fold() final;
  [[nodiscard]] bool check() const final;
  [[nodiscard]] Type type() const final;
    [[nodiscard]] bool constant() const final;
  void visit(IRVisitor& visitor) final;

    PrefixOperation                operation;
    std::variant<Variable*, Expr*> operand;
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
  [[nodiscard]] Literal* fold() final;
  [[nodiscard]] bool check() const final;
  [[nodiscard]] Type type() const final;
    [[nodiscard]] bool constant() const final;
  void visit(IRVisitor& visitor) final;

    Type  cast;
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
  [[nodiscard]] Literal* fold() final;
  [[nodiscard]] bool check() const final;
  [[nodiscard]] Type type() const final;
    [[nodiscard]] bool constant() const final;
  void visit(IRVisitor& visitor) final;

    Variable* variable;
    Expr*     expr;
};

struct FunctionCall final : public Expr
{
    FunctionCall(std::vector<Expr*> arguments, std::string identifier, std::shared_ptr<SymbolTable> table, size_t line, size_t column)
    : Expr(std::move(table), line, column), arguments(std::move(arguments)), identifier(std::move(identifier)) {}

    [[nodiscard]] std::string        name() const final;
    [[nodiscard]] std::string        value() const final;
    [[nodiscard]] std::vector<Node*> children() const final;
    [[nodiscard]] Literal*           fold() final;
    [[nodiscard]] bool               check() const final;
    [[nodiscard]] Type               type() const final;
    [[nodiscard]] bool constant() const final;
	void visit(IRVisitor& visitor) final;

	std::vector<Expr*> arguments;
    std::string identifier;
};

struct PrintfStatement final : public Expr
{
    explicit PrintfStatement(Expr* expr, std::shared_ptr<SymbolTable> table, size_t line, size_t column)
    : Expr(std::move(table), line, column), expr(expr)
    {
    }

    [[nodiscard]] std::string        name() const final;
    [[nodiscard]] std::string        value() const final;
    [[nodiscard]] std::vector<Node*> children() const final;
    [[nodiscard]] Literal* fold() final;
    [[nodiscard]] Type type() const final;
    [[nodiscard]] bool constant() const final;
	void visit(IRVisitor& visitor) override;

    Expr* expr;
};

} // namespace Ast
