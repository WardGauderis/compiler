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

    [[nodiscard]] std::string        name() const final;
    [[nodiscard]] std::vector<Node*> children() const final;
    [[nodiscard]] std::string        color() const final;
    [[nodiscard]] Node*              fold() final;

    void visit(IRVisitor& visitor) override;

    std::vector<Statement*> statements;
};

struct VariableDeclaration final : public Statement
{
    explicit VariableDeclaration(Type*                        type,
                                 std::string                  identifier,
                                 Expr*                        expr,
                                 std::shared_ptr<SymbolTable> table,
                                 size_t                       line,
                                 size_t                       column)
    : Statement(std::move(table), line, column), type(std::move(type)),
      identifier(std::move(identifier)), expr(expr)
    {
    }

    [[nodiscard]] std::string        name() const final;
    [[nodiscard]] std::string        value() const final;
    [[nodiscard]] std::vector<Node*> children() const final;
    [[nodiscard]] Node*              fold() final;
    [[nodiscard]] bool               fill() const final;
    [[nodiscard]] bool               check() const final;
    void                             visit(IRVisitor& visitor) final;

    Type*       type;
    std::string identifier;
    Expr*       expr; // can be nullptr
};

struct FunctionDefinition : public Statement
{
    FunctionDefinition(Type*                                      returnType,
                       std::string                                identifier,
                       std::vector<std::pair<Type*, std::string>> parameters,
                       Scope*                                     body,
                       std::shared_ptr<SymbolTable>               table,
                       size_t                                     line,
                       size_t                                     column)
    : returnType(returnType), identifier(std::move(identifier)), parameters(std::move(parameters)),
      body(body), Statement(std::move(table), line, column)
    {
    }

    [[nodiscard]] std::string        name() const final;
    [[nodiscard]] std::string        value() const final;
    [[nodiscard]] std::vector<Node*> children() const final;
    [[nodiscard]] Node*              fold() final;
    [[nodiscard]] bool               fill() const final;
    [[nodiscard]] bool               check() const final;
    void                             visit(IRVisitor& visitor) final;

    Type*                                      returnType;
    std::string                                identifier;
    std::vector<std::pair<Type*, std::string>> parameters;
    Scope*                                     body;
};

struct FunctionDeclaration : public Statement
{
    FunctionDeclaration(Type*                                      returnType,
                        std::string                                identifier,
                        std::vector<std::pair<Type*, std::string>> parameters,
                        std::shared_ptr<SymbolTable>               table,
                        size_t                                     line,
                        size_t                                     column)
    : returnType(returnType), identifier(std::move(identifier)), parameters(std::move(parameters)),
      Statement(std::move(table), line, column)
    {
    }

    [[nodiscard]] std::string name() const final;
    [[nodiscard]] std::string value() const final;
    [[nodiscard]] Node*       fold() final;
    [[nodiscard]] bool        fill() const final;
    void                      visit(IRVisitor& visitor) final;

    Type*                                      returnType;
    std::string                                identifier;
    std::vector<std::pair<Type*, std::string>> parameters;
};

struct LoopStatement final : public Statement
{
    explicit LoopStatement(std::vector<Statement*>      init,
                           Expr*                        condition,
                           Expr*                        iteration,
                           Statement*                   body,
                           bool                         doWhile,
                           std::shared_ptr<SymbolTable> table,
                           size_t                       line,
                           size_t                       column)
    : Statement(std::move(table), line, column), init(std::move(init)), condition(condition),
      iteration(iteration), body(body), doWhile(doWhile)
    {
    }

    [[nodiscard]] std::string        name() const final;
    [[nodiscard]] std::vector<Node*> children() const final;
    [[nodiscard]] Node*              fold() final;
    void                             visit(IRVisitor& visitor) final;

    std::vector<Statement*> init;
    Expr*                   condition; // can be nullptr
    Expr*                   iteration; // can be nullptr
    Statement*              body;
    bool                    doWhile;
};

struct IfStatement final : public Statement
{
    explicit IfStatement(Expr*                        condition,
                         Statement*                   ifBody,
                         Statement*                   elseBody,
                         std::shared_ptr<SymbolTable> table,
                         size_t                       line,
                         size_t                       column)
    : Statement(std::move(table), line, column), condition(condition), ifBody(ifBody), elseBody(elseBody)
    {
    }

    [[nodiscard]] std::string        name() const final;
    [[nodiscard]] std::vector<Node*> children() const final;
    [[nodiscard]] Node*              fold() final;
    void                             visit(IRVisitor& visitor) final;

    Expr*      condition;
    Statement* ifBody;
    Statement* elseBody; // can be nullptr
};

struct ControlStatement final : public Statement
{
    explicit ControlStatement(std::string type, std::shared_ptr<SymbolTable> table, size_t line, size_t column)
    : Statement(std::move(table), line, column), type(std::move(type))
    {
    }

    [[nodiscard]] std::string name() const final;
    [[nodiscard]] bool        check() const final;
    [[nodiscard]] Node*       fold() final;
    void                      visit(IRVisitor& visitor) final;

    std::string type;
};

struct ReturnStatement final : public Statement
{
    explicit ReturnStatement(Expr* expr, std::shared_ptr<SymbolTable> table, size_t line, size_t column)
    : Statement(std::move(table), line, column), expr(expr)
    {
    }

    [[nodiscard]] std::string        name() const final;
    [[nodiscard]] std::vector<Node*> children() const final;
    [[nodiscard]] bool               check() const final;
    [[nodiscard]] Node*              fold() final;
    void                             visit(IRVisitor& visitor) final;

    Expr* expr; // can be nullptr
};

struct IncludeStdioStatement final : public Statement
{
    explicit IncludeStdioStatement(std::shared_ptr<SymbolTable> table, size_t line, size_t column)
    : Statement(std::move(table), line, column)
    {
    }

    [[nodiscard]] std::string        name() const final;
    [[nodiscard]] std::vector<Node*> children() const final;
    [[nodiscard]] bool               check() const final;
    [[nodiscard]] bool               fill() const final;
    [[nodiscard]] Node*              fold() final;
    void                             visit(IRVisitor& visitor) final;
};

} // namespace Ast