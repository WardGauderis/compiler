//============================================================================
// @author      : Thomas Dooms
// @date        : 5/10/20
// @copyright   : BA2 Informatica - Thomas Dooms - University of Antwerp
//============================================================================

#include "mips.h"
#include <llvm/IR/Type.h>

namespace
{
std::string reg(uint num)
{
    return "$" + std::to_string(num);
}

template <typename... Args>
std::string operation(const std::string& operation, const Args&... args)
{
    if constexpr(not std::is_same_v<Args..., std::string>)
    {
        throw std::logic_error("types must all be string");
    }

    std::string res = (operation + ' ');
    res += ((args + ','), ...);
    res.back() = '\n';
    return res;
}
} // namespace

namespace mips
{
std::string reg(uint num)
{
    return "$" + std::to_string(num);
}

template <typename... Args>
std::string operation(const std::string& operation, const Args&... args)
{
    if constexpr(not std::conjunction_v<std::is_same<Args, std::string>...>)
    {
        throw std::logic_error("types must all be string");
    }

    std::string res = (operation + ' ');
    res += ((args + ','), ...);
    res.back() = '\n';
    return res;
}

bool isFloat(llvm::Value* value)
{
    const auto type = value->getType();

    if(type->isFloatTy())
    {
        return true;
    }
    else if(type->isIntegerTy())
    {
        return false;
    }
    else if(type->isPointerTy())
    {
        return false;
    }
    else
    {
        throw std::logic_error("unsupported type");
    }
}


void assertSame(llvm::Value* val1, llvm::Value* val2)
{
    if(isFloat(val1) != isFloat(val2))
    {
        throw std::logic_error("types do not have same type class");
    }
}

void assertSame(llvm::Value* val1, llvm::Value* val2, llvm::Value* t3)
{
    if(isFloat(val1) != isFloat(val2) or isFloat(val1) != isFloat(t3))
    {
        throw std::logic_error("types do not have same type class");
    }
}

void assertInt(llvm::Value* value)
{
    if(isFloat(value))
    {
        throw std::logic_error("type must be integer");
    }
}

void assertFloat(llvm::Value* value)
{
    if(not isFloat(value))
    {
        throw std::logic_error("type must be float");
    }
}

uint RegisterMapper::loadValue(std::string& output, llvm::Value* id)
{
    if(id == nullptr)
    {
        throw std::logic_error("register value id cannot be nullptr");
    }

    const auto index = isFloat(id);
    const auto regIter = registerDescriptors[index].find(id);

    if(regIter == registerDescriptors[index].end())
    {
        const auto addrIter = addressDescriptors[index].find(id);

        // if no register is available: spill something else into memory
        if(emptyRegisters[index].empty())
        {
            storeValue(output, id);

            const auto res = nextSpill[index];
            nextSpill[index] = (nextSpill[index] + 1) % registerSize[index];
            return res;
        }
        else
        {
            const auto res = emptyRegisters[index].back();
            emptyRegisters[index].pop_back();

            // if address is found: load word from the memory and remove the address entry
            if(addrIter != addressDescriptors[index].end())
            {
                output += operation("lw", reg(res), std::to_string(addrIter->second) + "($sp)");
                addressDescriptors[index].erase(addrIter);
                registerDescriptors[index].emplace(id, res);
            }

            return res;
        }
    }
    else
    {
        return regIter->second;
    }
}

void RegisterMapper::storeValue(std::string& output, llvm::Value* id)
{
    if(id == nullptr)
    {
        throw std::logic_error("register value id cannot be nullptr");
    }

    const auto index = isFloat(id);
    const auto iter = registerDescriptors[index].find(id);

    if(iter != registerDescriptors[index].end())
    {
        output += operation("sw", reg(iter->second), std::to_string(stackSize) + "($sp)");
        stackSize += 4;

        emptyRegisters[index].push_back(iter->second);
        registerDescriptors[index].emplace(id, iter->second);
    }
    else
    {
        throw std::logic_error("cannot store unused register");
    }
}

Move::Move(llvm::Value* t1, llvm::Value* t2)
{
    assertSame(t1, t2);

    const auto index1 = mapper->loadValue(output, t1);
    const auto index2 = mapper->loadValue(output, t2);
    output += operation(isFloat(t1) ? "mov.s" : "move", reg(index1), reg(index2));
}

Load::Load(llvm::Value* t1, llvm::Value* t2, int offset)
{
    const auto index1 = mapper->loadValue(output, t1);
    if(isFloat(t1))
    {
        const bool isWord = t1->getType()->getIntegerBitWidth() == 32;
        output += operation(isWord ? "lw" : "lb", std::to_string(offset) + '(' + reg(index1) +')');
    }
    else
    {
        // TODO
    }
}

Load::Load(llvm::Value* t1, int value)
{
    const auto index1 = mapper->loadValue(output, t1);
    output += operation("lui", reg(index1), std::to_string(value & 0xffff0000u));
    output += operation("ori", reg(index1), std::to_string(value & 0x0000ffffu));
}

Load::Load(llvm::Value* t1, float value)
{
    // TODO
}

Load::Load(llvm::Value* t1, std::string label)
{
    const auto index1 = mapper->loadValue(output, t1);
    if(isFloat(t1))
    {
        const bool isWord = t1->getType()->getIntegerBitWidth() == 32;
        output += operation(isWord ? "lw" : "lb", reg(index1), label);
    }
    else
    {
        // TODO
    }
}

Arithmetic::Arithmetic(std::string type, llvm::Value* t1, llvm::Value* t2, llvm::Value* t3)
{
    const auto index1 = mapper->loadValue(output, t1);
    const auto index2 = mapper->loadValue(output, t2);
    const auto index3 = mapper->loadValue(output, t3);

    if(isFloat(t1))
    {
        output += operation(type + ".s", reg(index1), reg(index2), reg(index3));
    }
    else
    {
        output += operation(type + "u", reg(index1), reg(index2), reg(index3));
    }
}

Arithmetic::Arithmetic(std::string type, llvm::Value* t1, llvm::Value* t2, int immediate)
{
    // TODO: brol als immediate boven 2^16 is
    const auto index1 = mapper->loadValue(output, t1);
    const auto index2 = mapper->loadValue(output, t2);
    output += operation(type + "iu", reg(index1), reg(index2), std::to_string(immediate));
}

Modulo::Modulo(llvm::Value* t1, llvm::Value* t2, llvm::Value* t3 )
{
    const auto index1 = mapper->loadValue(output, t1);
    const auto index2 = mapper->loadValue(output, t2);
    const auto index3 = mapper->loadValue(output, t3);

    if(isFloat(t1))
    {
        output += operation("div.s", reg(index1), reg(index2), reg(index3));
    }
    else
    {
        output += operation("divu", reg(index2), reg(index3));
        output += operation("mfhi", reg(index1));
    }
}

Comparison::Comparison(const std::string& type, llvm::Value* t1, llvm::Value* t2, llvm::Value* t3)
{
    const auto index1 = mapper->loadValue(output, t1);
    const auto index2 = mapper->loadValue(output, t2);
    const auto index3 = mapper->loadValue(output, t3);

    output += operation(type, reg(index1), reg(index2), reg(index3));
}

Branch::Branch(std::string type, llvm::Value *t1, llvm::Value *t2, std::string label)
{
    const auto index1 = mapper->loadValue(output, t1);
    const auto index2 = mapper->loadValue(output, t2);

    output += operation(type, reg(index1), reg(index2), label);
}

Call::Call(std::string label)
{
    output += operation("jal", label);
}

Jump::Jump(std::string label)
{
    output += operation("j", label);
}

Store::Store(llvm::Value* t1, llvm::Value* t2, uint offset)
{
    const auto isWord = t1->getType()->getIntegerBitWidth() == 32;
    const auto index1 = mapper->loadValue(output, t1);
    const auto index2 = mapper->loadValue(output, t2);
    output += operation(isWord ? "sw" : "sb", reg(index1), std::to_string(offset) + '(' + reg(index2) + ')');
}

Store::Store(llvm::Value* t1, std::string label, uint offset)
{
    const auto isWord = t1->getType()->getIntegerBitWidth() == 32;
    const auto index1 = mapper->loadValue(output, t1);
    output += operation(isWord ? "sw" : "sb", reg(index1), label + '+' + std::to_string(offset));
}

void Block::append(Instruction* instruction)
{
    instructions.emplace_back(instruction);
}

void Block::print(std::ostream& os) const
{
    os << label << ":\n";
    for(const auto& instruction : instructions)
    {
        instruction->print(os);
    }
}


void Function::append(Block* block)
{
    block->mapper = mapper;
    blocks.emplace_back(block);
}

void Function::print(std::ostream& os) const
{
    for(const auto& block : blocks)
    {
        block->print(os);
    }
}


void Module::append(Function* function)
{
    functions.emplace_back(function);
}

void Module::print(std::ostream& os) const
{
    for(const auto& function : functions)
    {
        function->print(os);
    }
}


} // namespace mips
