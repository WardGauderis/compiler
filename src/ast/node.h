//============================================================================
// @author      : Thomas Dooms
// @date        : 3/21/20
// @copyright   : BA2 Informatica - Thomas Dooms - University of Antwerp
//============================================================================

#pragma once

#include "../table.h"
#include <filesystem>
#include <llvm/IR/Value.h>

class IRVisitor;

namespace Ast
{

struct Literal;

struct Node
{
    explicit Node(std::shared_ptr<SymbolTable> table, size_t line, size_t column)
    : table(std::move(table)), column(column), line(line)
    {
    }

    friend std::ofstream& operator<<(std::ofstream& stream, const std::unique_ptr<Node>& root);

    void complete(bool check, bool fold, bool output);

    [[nodiscard]] virtual std::string name() const = 0;

    [[nodiscard]] virtual std::string value() const;

    [[nodiscard]] virtual std::vector<Node*> children() const;

    [[nodiscard]] virtual std::string color() const = 0;

    [[nodiscard]] virtual Node* fold() = 0;

    [[nodiscard]] virtual bool fill() const;

    [[nodiscard]] virtual bool check() const;

    virtual void visit(IRVisitor& visitor) = 0;

    size_t column;
    size_t line;

    std::shared_ptr<SymbolTable> table;
};

struct Comment final : public Node
{
    explicit Comment(std::string comment, std::shared_ptr<SymbolTable> table, size_t line, size_t column)
    : Node(std::move(table), line, column), comment(std::move(comment))
    {
    }

    [[nodiscard]] std::string name() const final;
    [[nodiscard]] std::string value() const final;
    [[nodiscard]] std::string color() const final;
    [[nodiscard]] Node* fold() final{ return nullptr; }
    void                      visit(IRVisitor& visitor) final;

    std::string comment;
};

struct Statement : public Node
{
    explicit Statement(std::shared_ptr<SymbolTable> table, size_t line, size_t column)
    : Node(std::move(table), line, column)
    {
    }

    [[nodiscard]] std::string color() const override;
};

void ast2ir(const std::unique_ptr<Ast::Node>& root,
            const std::filesystem::path&      input,
            const std::filesystem::path&      output,
            bool                              optimised);

} // namespace Ast