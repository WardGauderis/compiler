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

struct Type
{
    explicit Type(bool isConst, Type* ptr) : isConst(isConst), type(ptr)
    {
    }

    explicit Type(bool isConst, std::string base) : isConst(isConst), type(std::move(base))
    {
    }

    [[nodiscard]] std::string print() const
    {
        return std::visit(overloaded
        {
            [&](Type* type)
            {
                return type->print() + "*" + (isConst ? " const" : "");
            },
            [&](const std::string& name)
            {
                return (isConst ? "const " : "") + name;
            }
        }, type);
    }

    bool isConst;
    std::variant<Type*, std::string> type;
};

class SymbolTable
{
public:
    using Table = std::unordered_map<std::string, std::pair<Type*, std::optional<base_type>>>;
    using Entry = Table::const_iterator;

    explicit SymbolTable(std::shared_ptr<SymbolTable> parent = nullptr) : parent(std::move(parent))
    {
    }

    Entry lookup(const std::string& id) const
    {
        const auto iter = table.find(id);
        if(iter == table.end())
        {
            if(parent) return parent->lookup(id);
            else throw WhoopsiePoopsieError("could not find element: " + id + " in any symbol table");
        }
        else return iter;
    }

    void set_literal(const std::string& id, std::optional<base_type> type)
    {
        const auto& iter = table.find(id);
        if(iter == table.end()) throw WhoopsiePoopsieError("settings literal for unknown element");
        else iter->second.second = type;
    }


    Entry insert(const std::string& id, Type* type)
    {
        const auto [iter, inserted] = table.emplace(id, std::make_pair(type, std::nullopt));
        if(not inserted) throw SyntaxError("already declared symbol with id: " + id);
        else return iter;
    }

private:
    std::shared_ptr<SymbolTable> parent;
    Table table;
};
