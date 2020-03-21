//============================================================================
// @author      : Thomas Dooms
// @date        : 3/21/20
// @copyright   : BA2 Informatica - Thomas Dooms - University of Antwerp
//============================================================================

#pragma once

#include "node.h"
#include "expressions.h"

namespace Ast
{

struct Declaration final : public Statement
{
  explicit Declaration(Type vartype, Variable* variable, Expr* expr, std::shared_ptr<SymbolTable> table, size_t line, size_t column)
      : Statement(std::move(table), line, column), vartype(vartype), variable(variable), expr(expr)
  {
  }

  [[nodiscard]] std::string name() const final;
  [[nodiscard]] std::string value() const final;
  [[nodiscard]] std::vector<Node*> children() const final;
  [[nodiscard]] Literal* fold() final;
  [[nodiscard]] bool check() const final;
  [[nodiscard]] llvm::Value* codegen() const final;

  Type vartype;
  Variable* variable;
  Expr* expr; // can be nullptr
};

struct LoopStatement final : public Statement
{
  explicit LoopStatement(Declaration* declaration, Expr* condition, Expr* iteration, Block* body, std::shared_ptr<SymbolTable> table, size_t line, size_t column)
      : Statement(std::move(table), line, column), declaration(declaration), condition(condition), iteration(iteration), body(body)
  {
  }

  [[nodiscard]] std::string name() const final;
  [[nodiscard]] std::string value() const final;
  [[nodiscard]] std::vector<Node*> children() const final;
  [[nodiscard]] Literal* fold() final;
  [[nodiscard]] bool check() const final;
  [[nodiscard]] llvm::Value* codegen() const final;

  Declaration* declaration; // can be nullptr
  Expr* condition; // can be nullptr
  Expr* iteration; // can be nullptr
  Block* body;
};

struct IfStatement final : public Statement
{
  explicit IfStatement(Expr* condition, Block* body, std::shared_ptr<SymbolTable> table, size_t line, size_t column)
      : Statement(std::move(table), line, column), condition(condition), body(body)
  {
  }

  [[nodiscard]] std::string name() const final;
  [[nodiscard]] std::string value() const final;
  [[nodiscard]] std::vector<Node*> children() const final;
  [[nodiscard]] Literal* fold() final;
  [[nodiscard]] bool check() const final;
  [[nodiscard]] llvm::Value* codegen() const final;

  Expr* condition;
  Block* body;
};

struct CaseStatement final : public Statement
{
  explicit CaseStatement(Literal* literal, Block* body, std::shared_ptr<SymbolTable> table, size_t line, size_t column)
      : Statement(std::move(table), line, column), literal(literal), body(body)
  {
  }

  [[nodiscard]] std::string name() const final;
  [[nodiscard]] std::string value() const final;
  [[nodiscard]] std::vector<Node*> children() const final;
  [[nodiscard]] Literal* fold() final;
  [[nodiscard]] bool check() const final;
  [[nodiscard]] llvm::Value* codegen() const final;

  Literal* literal; // nullptr will mean default case
  Block* body;
};

struct SwitchStatement final : public Statement
{
  explicit SwitchStatement(Variable* switchVar, std::vector<CaseStatement*> cases, std::shared_ptr<SymbolTable> table, size_t line, size_t column)
      : Statement(std::move(table), line, column), switchVar(switchVar), cases(std::move(cases))
  {
  }

  [[nodiscard]] std::string name() const final;
  [[nodiscard]] std::string value() const final;
  [[nodiscard]] std::vector<Node*> children() const final;
  [[nodiscard]] Literal* fold() final;
  [[nodiscard]] bool check() const final;
  [[nodiscard]] llvm::Value* codegen() const final;

  Variable* switchVar;
  std::vector<CaseStatement*> cases;
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
  [[nodiscard]] Literal* fold() final;
  [[nodiscard]] llvm::Value* codegen() const final;

  Expr* expr;
};

}