//============================================================================
// @author      : Thomas Dooms
// @date        : 5/10/20
// @copyright   : BA2 Informatica - Thomas Dooms - University of Antwerp
//============================================================================

#include "mips.h"
#include "../errors.h"
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Type.h>

namespace
{
std::string reg(uint num)
{
    return "$" + std::to_string(num);
}

template <typename Ptr>
std::string label(Ptr* ptr)
{
    return std::to_string(reinterpret_cast<size_t>(ptr)); // nobody can see this line
}

template <typename... Args>
std::string operation(std::string&& operation, Args&&... args)
{
    if constexpr(not std::conjunction_v<std::is_same<Args, std::string>...>)
    {
        throw InternalError("types must all be string");
    }
    else
    {
        std::string res = (operation + ' ');
        res += ((args + ','), ...);
        res.back() = '\n';
        return res;
    }
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
        throw InternalError("unsupported type for mips");
    }
}


void assertSame(llvm::Value* val1, llvm::Value* val2)
{
    if(isFloat(val1) != isFloat(val2))
    {
        throw InternalError("types do not have same type class");
    }
}

void assertSame(llvm::Value* val1, llvm::Value* val2, llvm::Value* t3)
{
    if(isFloat(val1) != isFloat(val2) or isFloat(val1) != isFloat(t3))
    {
        throw InternalError("types do not have same type class");
    }
}

void assertInt(llvm::Value* value)
{
    if(isFloat(value))
    {
        throw InternalError("type must be integer");
    }
}

void assertFloat(llvm::Value* value)
{
    if(not isFloat(value))
    {
        throw InternalError("type must be float");
    }
}
} // namespace

namespace mips
{


uint RegisterMapper::loadValue(std::string& output, llvm::Value* id)
{
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
        throw InternalError("cannot store unused register");
    }
}

uint RegisterMapper::getSize() const noexcept
{
    return stackSize;
}

void Instruction::print(std::ostream& os)
{
    os << output;
}

void Instruction::setMapper(std::shared_ptr<RegisterMapper> imapper)
{
    mapper = std::move(imapper);
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
        throw InternalError("TODO loat fload");
        // TODO
    }
    else
    {
        const bool isWord = t1->getType()->getIntegerBitWidth() == 32;
        output += operation(isWord ? "lw" : "lb", std::to_string(offset) + '(' + reg(index1) + ')');
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
    throw InternalError("TODO floadt immediate");
    // TODO
    // mtc1
}

Load::Load(llvm::Value* t1, llvm::GlobalVariable* variable)
{
    const auto index1 = mapper->loadValue(output, t1);
    if(isFloat(t1))
    {
        output += operation("l.s", reg(index1), label(variable));
    }
    else
    {
        const bool isWord = t1->getType()->getIntegerBitWidth() == 32;
        output += operation(isWord ? "lw" : "lb", reg(index1), label(variable));
    }
}

Arithmetic::Arithmetic(std::string type, llvm::Value* t1, llvm::Value* t2, llvm::Value* t3)
{
    const auto index1 = mapper->loadValue(output, t1);
    const auto index2 = mapper->loadValue(output, t2);
    const auto index3 = mapper->loadValue(output, t3);

    if(isFloat(t1))
    {
        output += operation(std::move(type) + ".s", reg(index1), reg(index2), reg(index3));
    }
    else
    {
        output += operation(std::move(type) + "u", reg(index1), reg(index2), reg(index3));
    }
}

Arithmetic::Arithmetic(std::string type, llvm::Value* t1, llvm::Value* t2, int immediate)
{
    // TODO: brol als immediate boven 2^16 is
    const auto index1 = mapper->loadValue(output, t1);
    const auto index2 = mapper->loadValue(output, t2);
    output += operation(type + "iu", reg(index1), reg(index2), std::to_string(immediate));
}

Modulo::Modulo(llvm::Value* t1, llvm::Value* t2, llvm::Value* t3)
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

Comparison::Comparison(std::string type, llvm::Value* t1, llvm::Value* t2, llvm::Value* t3)
{
    const auto index1 = mapper->loadValue(output, t1);
    const auto index2 = mapper->loadValue(output, t2);
    const auto index3 = mapper->loadValue(output, t3);

    output += operation(std::move(type), reg(index1), reg(index2), reg(index3));
}

NotEquals::NotEquals(llvm::Value* t1, llvm::Value* t2, llvm::Value* t3)
{
    const auto index1 = mapper->loadValue(output, t1);
    const auto index2 = mapper->loadValue(output, t2);
    const auto index3 = mapper->loadValue(output, t3);

    output += operation("c.eq.s", reg(index1), reg(index2), reg(index3));
    output += operation("cmp", reg(index1), reg(index1), reg(0));
}

Branch::Branch(std::string type, llvm::Value* t1, llvm::Value* t2, llvm::BasicBlock* block)
{
    const auto index1 = mapper->loadValue(output, t1);
    const auto index2 = mapper->loadValue(output, t2);

    output += operation(std::move(type), reg(index1), reg(index2), label(block));
}

Call::Call(llvm::Function* function)
{
    output += operation("jal", label(function));
}

Jump::Jump(llvm::BasicBlock* block)
{
    output += operation("j", label(block));
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
    output += operation(isWord ? "sw" : "sb", reg(index1), std::move(label) + '+' + std::to_string(offset));
}

void Block::append(Instruction* instruction)
{
    instruction->setMapper(mapper);
    instructions.emplace_back(instruction);
}

void Block::appendBeforeLast(Instruction* instruction)
{
    instruction->setMapper(mapper);
    instructions.emplace(instructions.end() - 2, instruction);
}

void Block::print(std::ostream& os) const
{
    os << label(block) << ":\n";
    for(const auto& instruction : instructions)
    {
        instruction->print(os);
    }
}

llvm::BasicBlock* Block::getBlock()
{
    return block;
}

void Block::setMapper(std::shared_ptr<RegisterMapper> imapper)
{
    for(auto& instruction : instructions)
    {
        instruction->setMapper(imapper);
    }
    mapper = std::move(imapper);
}


void Function::append(Block* block)
{
    block->setMapper(mapper);
    blocks.emplace_back(block);
}

void Function::print(std::ostream& os) const
{
    os << label(function) << ":\n";
    os << operation("addi", "$sp", "$sp", std::to_string(-mapper->getSize()));
    for(const auto& block : blocks)
    {
        block->print(os);
    }
    os << operation("addi", "$sp", "$sp", std::to_string(mapper->getSize()));
}

Block* Function::getBlockByBasicBlock(llvm::BasicBlock* block)
{
    const auto pred = [&](const auto& ptr) { return ptr->getBlock() == block; };
    const auto iter = std::find_if(blocks.begin(), blocks.end(), pred);
    return (iter == blocks.end()) ? nullptr : iter->get();
}


void Module::append(Function* function)
{
    functions.emplace_back(function);
}

void Module::print(std::ostream& os) const
{
    os << ".data\n";

    os << ".text\n";
    for(const auto& function : functions)
    {
        function->print(os);
    }
}

void Module::addGlobal(llvm::GlobalVariable* variable)
{

}


} // namespace mips
