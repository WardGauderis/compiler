//============================================================================
// @author      : Thomas Dooms
// @date        : 3/15/20
// @copyright   : BA2 Informatica - Thomas Dooms - University of Antwerp
//============================================================================

#pragma once

#include "errors.h"
#include <memory>
#include <unordered_map>
#include <variant>

namespace
{
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;
}

using ptr_type = std::uint32_t;
using base_type = std::variant<char, short, int, long, float, double, ptr_type>;
using void_ptr = ptr_type;

struct Type
{
    explicit Type(bool isConst, Type* ptr) : isConst(isConst), type(ptr)
    {
    }

    explicit Type(bool isConst, std::string base) : isConst(isConst), type(std::move(base))
    {
    }

    [[nodiscard]] std::string print() const;

    bool isConst;
    std::variant<Type*, std::string, void_ptr> type;
};

struct TableElement
{
    Type* type;
    std::optional<base_type> literal;
};

class SymbolTable
{
public:
    using Table = std::unordered_map<std::string, TableElement>;
    using Entry = Table::const_iterator;

    explicit SymbolTable(std::shared_ptr<SymbolTable> parent = nullptr)
    : parent(std::move(parent))
    {
    }

    std::optional<Entry> lookup(const std::string& id) const;

    Entry insert(const std::string& id, Type* type);

    void set_literal(const std::string& id, std::optional<base_type> type);

    std::optional<base_type> get_literal(const std::string& id);

private:
    std::shared_ptr<SymbolTable> parent;
    Table table;
};
