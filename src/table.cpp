//============================================================================
// @author      : Thomas Dooms
// @date        : 3/15/20
// @copyright   : BA2 Informatica - Thomas Dooms - University of Antwerp
//============================================================================

#include "table.h"

std::optional<SymbolTable::Entry> SymbolTable::lookup(const std::string& id)
{
    auto iter = table.find(id);
    if (iter == table.end())
    {
        if (parent) return parent->lookup(id);
        else return std::nullopt;
    }
    else return iter;
}

std::pair<SymbolTable::Entry, bool> SymbolTable::insert(const std::string& id, Type type, bool initialized)
{
    return table.emplace(id, TableElement {type, std::nullopt, initialized});
}

void SymbolTable::set_initialized(const std::string& id)
{
    const auto& temp = lookup(id);
    if(temp.has_value()) (*temp)->second.initialized = true;
}

void SymbolTable::set_literal(const std::string& id, std::optional<TypeVariant> literal)
{
    const auto& temp = lookup(id);
    if (temp.has_value()) (*temp)->second.literal = literal;
    else throw InternalError("setting literal for unknown element");
}

std::optional<TypeVariant> SymbolTable::get_literal(const std::string& id)
{
    const auto& temp = lookup(id);
    if (temp.has_value()) return (*temp)->second.literal;
    else throw InternalError("getting literal for unknown element");
}

bool SymbolTable::lookup_initialized(const std::string& id)
{
    const auto temp = lookup(id);
    if(temp.has_value()) return (*temp)->second.initialized;
    else return false;
}

bool SymbolTable::lookup_const(const std::string& id)
{
    const auto temp = lookup(id);
    if(temp.has_value()) return (*temp)->second.type.isConst();
    else return false;
}
