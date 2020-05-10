//============================================================================
// @author      : Thomas Dooms
// @date        : 5/10/20
// @copyright   : BA2 Informatica - Thomas Dooms - University of Antwerp
//============================================================================

#include "mips.h"
#include <llvm/IR/Type.h>
#include <llvm/IR/DerivedTypes.h>

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
    if constexpr(not std::is_same_v<Args..., std::string>)
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


// gets the register for the value, if it is not yet in a register put it in one.
uint RegisterMapper::getRegister(std::ostream& os, llvm::Value* id)
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
            // od the spillies ye
            return -1;
        }
        else
        {
            const auto res = emptyRegisters[index].back();
            emptyRegisters[index].pop_back();

            // if address is found: load word from the memory and remove the address entry
            if(addrIter != addressDescriptors[index].end())
            {
                os << "lw " << reg(res) << addrIter->second << '\n';
                addressDescriptors[index].erase(addrIter);
            }

            return res;
        }
    }
    else
    {
        return regIter->second;
    }
}

void RegisterMapper::popValue(llvm::Value* id)
{
    const auto index = isFloat(id);
    const auto iter = registerDescriptors[index].find(id);

    if(iter != registerDescriptors[index].end())
    {
        emptyRegisters[index].push_back(iter->second);
        registerDescriptors[index].erase(iter);
    }
    addressDescriptors[index].erase(id);
}


void Move::print(std::ostream& os) const
{
    const auto index1 = mapper->getRegister(os, t1);
    const auto index2 = mapper->getRegister(os, t2);

    os << (isFloat(t1) ? "mov.s " : "move ");
    os << reg(index1) << ',' << reg(index2) << '\n';
}


void Load::print(std::ostream& os) const 
{
    const auto index1 = mapper->getRegister(os, t1);
    if(immediate)
    {
        if(isFloat(t1))
        {
            // TODO: load float
        }
        else
        {
            os << "lui " << reg(index1) << (ivalue & 0xffff0000u) << '\n'; // fuck the warnings even more
            os << "ori " << reg(index1) << (ivalue & 0x0000ffffu) << '\n';
        }
    }
    else
    {
        std::string temp;
        if(not label.empty())
        {
            temp += label;
        }
        if(ivalue != 0)
        {
            temp += ("+" + std::to_string(ivalue));
        }
        if(t2 != nullptr)
        {
            const auto index2 = mapper->getRegister(os, t2);
            temp += ('(' + reg(index2) + ')');
        }

        if(isFloat(t1))
        {
            // TODO: more load floats
        }
        else
        {
            os << (t1->getType()->getIntegerBitWidth() == 32 ? "lw " : "lb ");
            os << temp << '\n';
        }
    }
}


void Arithmetic::print(std::ostream& os) const 
{
    const auto index1 = mapper->getRegister(os, t1);
    const auto index2 = mapper->getRegister(os, t2);

    if(isFloat(t1))
    {
        const auto index3 = mapper->getRegister(os, t3);
        os << operation << ".s " << reg(index1) << ',' + reg(index2) + ',' << reg(index3) << '\n';
    }
    else if(t3 == nullptr)
    {
        // TODO: brol als immediate boven 2^16 is
        os << operation << "iu " << reg(index1) + ',' + reg(index2) + ',' << std::to_string(immediate) << '\n';
    }
    else
    {
        const auto index3 = mapper->getRegister(os, t3);
        os << operation << "u " << reg(index1) + ',' + reg(index2) + ',' << reg(index3) << '\n';
    }
}

void Modulo::print(std::ostream& os) const 
{
    const auto index1 = mapper->getRegister(os, t1);
    const auto index2 = mapper->getRegister(os, t2);
    const auto index3 = mapper->getRegister(os, t3);

    if(isFloat(t1))
    {
        os << "div.s" << reg(index1) << ',' << reg(index2) << ',' << reg(index3) << '\n';
    }
    else
    {
        os << "divu " << reg(index2) << ',' << reg(index3) << '\n';
        os << "mfhi " << reg(index1) << '\n';
    }
}


void Comparison::print(std::ostream& os) const 
{
    const auto index1 = mapper->getRegister(os, t1);
    const auto index2 = mapper->getRegister(os, t2);
    const auto index3 = mapper->getRegister(os, t3);

    os << operation << ' ' << reg(index1) << ',' << reg(index2) << ',' << reg(index3) << '\n';
}

void Branch::print(std::ostream& os) const 
{
    const auto index1 = mapper->getRegister(os, t1);
    const auto index2 = mapper->getRegister(os, t2);

    os << operation << ' ' << reg(index1) << ',' << reg(index2) << ',' << label << '\n';
}


void Jal::print(std::ostream& os)
{
    os << (link ? "jal" : "j") << /* registers */ label << '\n';
}

void Store::print(std::ostream& os) const 
{
    os << 's' << (isCharacter ? 'b' : 'w') << std::endl;
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
