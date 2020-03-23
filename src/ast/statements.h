//============================================================================
// @author      : Thomas Dooms
// @date        : 3/21/20
// @copyright   : BA2 Informatica - Thomas Dooms - University of Antwerp
//============================================================================

#pragma once

#include "expressions.h"
#include "node.h"

namespace Ast
{

struct Scope final : public Statement
{
    explicit Scope(std::vector<Statement*> statements, std::shared_ptr<SymbolTable> table, size_t line, size_t column)
    : Statement(std::move(table), line, column), statements(std::move(statements))
    {
    }

    [[nodiscard]] std::string name() const final;
    [[nodiscard]] std::string value() const final;
    [[nodiscard]] std::vector<Node*> children() const final;
    [[nodiscard]] std::string color() const final;
    [[nodiscard]] Literal* fold() final;
    [[nodiscard]] llvm::Value* codegen() const final;

    std::vector<Statement*> statements;
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
    [[nodiscard]] Literal* fold() final;
    [[nodiscard]] bool check() const final;
    [[nodiscard]] llvm::Value* codegen() const final;

    Type vartype;
    Variable* variable;
    Expr* expr; // can be nullptr
};

struct LoopStatement final : public Statement
{
    explicit LoopStatement(Statement* init, // may only be declaration or expr
                           Expr* condition,
                           Expr* iteration,
                           Statement* body,
                           bool doWhile,
                           std::shared_ptr<SymbolTable> table,
                           size_t line,
                           size_t column)
    : Statement(std::move(table), line, column), init(init), condition(condition),
      iteration(iteration), body(body), doWhile(doWhile)
    {
    }


    [[nodiscard]] std::string name() const final;
    [[nodiscard]] std::string value() const final;
    [[nodiscard]] std::vector<Node*> children() const final;
    [[nodiscard]] Literal* fold() final;
    [[nodiscard]] llvm::Value* codegen() const final {}

    Statement* init; // can be nullptr
    Expr* condition; // can be nullptr
    Expr* iteration; // can be nullptr
    Statement* body;
    bool doWhile;
};

struct IfStatement final : public Statement
{
    explicit IfStatement(Expr* condition, Statement* ifBody, Statement* elseBody, std::shared_ptr<SymbolTable> table, size_t line, size_t column)
    : Statement(std::move(table), line, column), condition(condition), ifBody(ifBody), elseBody(elseBody)
    {
    }

    [[nodiscard]] std::string name() const final;
    [[nodiscard]] std::string value() const final;
    [[nodiscard]] std::vector<Node*> children() const final;
    [[nodiscard]] Literal* fold() final;
    [[nodiscard]] llvm::Value* codegen() const final {}

    Expr* condition;
    Statement* ifBody;
    Statement* elseBody; // can be nullptr
};

struct ControlFlowStatement final : public Statement
{
    explicit ControlFlowStatement(std::string type, std::shared_ptr<SymbolTable> table, size_t line, size_t column)
    : Statement(std::move(table), line, column), type(std::move(type))
    {
    }

    [[nodiscard]] std::string name() const final;
    [[nodiscard]] std::string value() const final;
    [[nodiscard]] std::vector<Node*> children() const final;
    [[nodiscard]] Literal* fold() final;
    [[nodiscard]] llvm::Value* codegen() const final {}

    std::string type;
};

} // namespace Ast