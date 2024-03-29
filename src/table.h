//============================================================================
// @author      : Thomas Dooms
// @date        : 3/15/20
// @copyright   : BA2 Informatica - Thomas Dooms - University of Antwerp
//============================================================================

#pragma once

#include "errors.h"
#include "type.h"
#include <llvm/IR/Instructions.h>
#include <memory>
#include <unordered_map>
#include <variant>

enum class ScopeType
{
    plain,
    loop,
    condition,
    global,
    function
};

struct TableElement
{
    Type*                      type;
    std::optional<TypeVariant> literal;
    bool                       isInitialized;
    bool                       isDerefed;
    bool                       isUsed;
    llvm::Value*               allocaInst{};
};

class SymbolTable
{
    public:
    using Table = std::unordered_map<std::string, TableElement>;

    explicit SymbolTable(ScopeType type, std::shared_ptr<SymbolTable> parent = nullptr)
    : type(type), parent(std::move(parent))
    {
    }

    TableElement* lookup(const std::string& id);

    llvm::Value** lookupAllocaInst(const std::string& id);

    void setUsed(const std::string& id);

    bool insert(const std::string& id, Type* type, bool initialized);

    std::shared_ptr<SymbolTable>& getParent();

    ScopeType getType();

    bool lookupType(ScopeType type);

    private:
    std::shared_ptr<SymbolTable> parent;
    Table                        table;
    ScopeType                    type;
};
