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
    Type                       type;
    std::optional<TypeVariant> literal;
    bool                       isInitialized;
};

class SymbolTable
{
    public:
    using Table = std::unordered_map<std::string, TableElement>;

    explicit SymbolTable(std::shared_ptr<SymbolTable> parent = nullptr) : parent(std::move(parent))
    {
    }

    TableElement* lookup(const std::string& id);

    bool insert(const std::string& id, Type type, bool initialized);

    std::shared_ptr<SymbolTable>& getParent() { return parent; }

    private:
    std::shared_ptr<SymbolTable> parent;
    Table                        table;
};
