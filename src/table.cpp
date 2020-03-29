//============================================================================
// @author      : Thomas Dooms
// @date        : 3/15/20
// @copyright   : BA2 Informatica - Thomas Dooms - University of Antwerp
//============================================================================

#include "table.h"

TableElement* SymbolTable::lookup(const std::string& id)
{
    const auto iter = table.find(id);
    if(iter == table.end())
    {
        if(parent) return parent->lookup(id);
        else return nullptr;
    }
    else return &iter->second;
}

bool SymbolTable::insert(const std::string& id, Type type, bool initialized)
{
    return table.emplace(id, TableElement{ type, std::nullopt, initialized }).second;
}