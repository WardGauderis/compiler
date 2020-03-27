//============================================================================
// @author      : Thomas Dooms
// @date        : 3/15/20
// @copyright   : BA2 Informatica - Thomas Dooms - University of Antwerp
//============================================================================

#pragma once

#include "errors.h"
#include "type.h"
#include <memory>
#include <unordered_map>
#include <variant>

struct TableElement
{
    Type type;
    std::optional<TypeVariant> literal;
    bool initialized;
};

class SymbolTable
{
public:
    using Table = std::unordered_map<std::string, TableElement>;
    using Entry = Table::iterator;

    explicit SymbolTable(std::shared_ptr<SymbolTable> parent = nullptr) : parent(std::move(parent))
    {
    }

    std::optional<Entry> lookup(const std::string& id);

    std::pair<Entry, bool> insert(const std::string& id, Type type, bool initialized);

    void set_initialized(const std::string& id);

    void set_literal(const std::string& id, std::optional<TypeVariant> type);

    std::optional<TypeVariant> get_literal(const std::string& id);

    bool lookup_initialized(const std::string& id);

    bool lookup_const(const std::string& id);

private:
    std::shared_ptr<SymbolTable> parent;
    Table table;
};
