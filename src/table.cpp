//============================================================================
// @author      : Thomas Dooms
// @date        : 3/15/20
// @copyright   : BA2 Informatica - Thomas Dooms - University of Antwerp
//============================================================================

#include "table.h"



std::optional<SymbolTable::Entry> SymbolTable::lookup(const std::string& id) const
{
    const auto iter = table.find(id);
    if(iter == table.end())
    {
        if(parent) return parent->lookup(id);
        else return std::nullopt;
    }
    else return iter;
}

SymbolTable::Entry SymbolTable::insert(const std::string& id, Type type)
{
    const auto [iter, inserted] = table.emplace(id, TableElement{type, std::nullopt});
    if(not inserted) throw SyntaxError("redefinition of '" + id + "'");
    else return iter;
}

void SymbolTable::set_literal(const std::string& id, std::optional<TypeVariant> literal)
{
    const auto& iter = table.find(id);
    if(iter == table.end()) throw WhoopsiePoopsieError("setting literal for unknown element");
    else iter->second.literal = literal;
}

std::optional<TypeVariant> SymbolTable::get_literal(const std::string& id)
{
    const auto& iter = table.find(id);
    if(iter == table.end()) throw WhoopsiePoopsieError("getting literal for unknown element");
    else return iter->second.literal;
}

bool SymbolTable::lookup_const(const std::string& id)
{
    return lookup(id).value()->second.type.isConst();
}


