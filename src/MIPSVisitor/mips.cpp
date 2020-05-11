//============================================================================
// @author      : Thomas Dooms
// @date        : 5/10/20
// @copyright   : BA2 Informatica - Thomas Dooms - University of Antwerp
//============================================================================

#include "mips.h"
#include "../errors.h"
#include <llvm/IR/Constants.h>
#include <llvm/IR/Type.h>

namespace
{
std::string reg(uint num)
{
    return (num >= 32 ? "$f" : "$") + std::to_string(num);
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
    else if(type->isIntegerTy() or type->isPointerTy())
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
    if(const auto& constant = llvm::dyn_cast<llvm::ConstantInt>(id))
    {
        const auto immediate = int(constant->getSExtValue());
        const auto res = getTemp();

        output += operation("lui", reg(res), std::to_string(immediate & 0xffff0000u));
        output += operation("ori", reg(res), std::to_string(immediate & 0x0000ffffu));
        return res;
    }
    else if(const auto& constant = llvm::dyn_cast<llvm::ConstantFP>(id))
    {
        module->addFloat(constant);
        const auto res = getTemp();

        output += operation("l.s", reg(res), label(id));
        return res;
    }

    const auto regIter = registerDescriptors[index].find(id);

    if(regIter == registerDescriptors[index].end())
    {
        const auto addrIter = addressDescriptors[index].find(id);

        // if no register is available: spill something else into memory
        if(emptyRegisters[index].empty())
        {
            storeValue(output, id);

            const auto temp = nextSpill[index] + 32 * index;
            nextSpill[index] = (nextSpill[index] + 1) % registerSize[index];
            return temp;
        }
        else
        {
            const auto res = emptyRegisters[index].back() + 32 * index;
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
        return regIter->second + 32 * index;
    }
}

uint RegisterMapper::getTemp()
{
    return (tempReg) ? tempReg-- : tempReg++;
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

void RegisterMapper::storeRegisters(std::string& output)
{
    for(const auto [id, reg] : registerDescriptors[0])
    {
        storeValue(output, id);
    }
    for(const auto [id, reg] : registerDescriptors[1])
    {
        storeValue(output, id);
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

void Instruction::setBlock(Block* b)
{
    block = b;
}

RegisterMapper* Instruction::mapper()
{
    return block->function->getMapper();
}

Module* Instruction::module()
{
    return block->function->getModule();
}

Move::Move(Block* block, llvm::Value* t1, llvm::Value* t2) : Instruction(block)
{
    assertSame(t1, t2);

    const auto index1 = mapper()->loadValue(output, t1);
    const auto index2 = mapper()->loadValue(output, t2);

    output += operation(isFloat(t1) ? "mov.s" : "move", reg(index1), reg(index2));
}

Convert::Convert(Block* block, llvm::Value* t1, llvm::Value* t2) : Instruction(block)
{
    // converts t2 into t1
    const auto index1 = mapper()->loadValue(output, t1);
    const auto index2 = mapper()->loadValue(output, t2);

    if(isFloat(t2))
    {
        // float to int
        assertInt(t1);

        output += operation("cvt.s.w", reg(index2), reg(index2));
        output += operation("mfc1", reg(index1), reg(index2));
    }
    else
    {
        // int to float
        assertFloat(t1);

        output += operation("mtc1", reg(index2), reg(index1));
        output += operation("cvt.w.s", reg(index1), reg(index1));
    }
}

Load::Load(Block* block, llvm::Value* t1, llvm::Value* t2) : Instruction(block)
{
    const auto index1 = mapper()->loadValue(output, t1);

    if(llvm::isa<llvm::Constant>(t2))
    {
    }
    else if(isFloat(t1))
    {
        const auto index2 = mapper()->loadValue(output, t2);

        output += operation("lw", reg(mapper()->getTemp()), reg(index2));
        output += operation("mtc1", reg(mapper()->getTemp()), reg(index1));
    }
    else
    {
        const auto index2 = mapper()->loadValue(output, t2);
        const bool isWord = t1->getType()->getIntegerBitWidth() == 32;

        output += operation(isWord ? "lw" : "lb", reg(index1), reg(index2));
    }
}

Load::Load(Block* block, llvm::Value* t1, llvm::GlobalVariable* variable) : Instruction(block)
{
    const auto index1 = mapper()->loadValue(output, t1);
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

Arithmetic::Arithmetic(Block* block, std::string type, llvm::Value* t1, llvm::Value* t2, llvm::Value* t3) : Instruction(block)
{
    const auto index1 = mapper()->loadValue(output, t1);
    const auto index2 = mapper()->loadValue(output, t2);
    const auto index3 = mapper()->loadValue(output, t3);

    output += operation(std::move(type), reg(index1), reg(index2), reg(index3));
}

Modulo::Modulo(Block* block, llvm::Value* t1, llvm::Value* t2, llvm::Value* t3) : Instruction(block)
{
    const auto index1 = mapper()->loadValue(output, t1);
    const auto index2 = mapper()->loadValue(output, t2);
    const auto index3 = mapper()->loadValue(output, t3);

    output += operation("divu", reg(index2), reg(index3));
    output += operation("mfhi", reg(index1));
}

NotEquals::NotEquals(Block* block, llvm::Value* t1, llvm::Value* t2, llvm::Value* t3) : Instruction(block)
{
    const auto index1 = mapper()->loadValue(output, t1);
    const auto index2 = mapper()->loadValue(output, t2);
    const auto index3 = mapper()->loadValue(output, t3);

    output += operation("c.eq.s", reg(index1), reg(index2), reg(index3));
    output += operation("cmp", reg(index1), reg(index1), reg(0));
}

Branch::Branch(Block* block, llvm::Value* t1, llvm::BasicBlock* target, bool eqZero) : Instruction(block)
{
    const auto index1 = mapper()->loadValue(output, t1);

    output += operation(eqZero ? "beqz" : "bnez", reg(index1), label(target));
}

Call::Call(Block* block, llvm::Function* function) : Instruction(block)
{
    mapper()->storeRegisters(output);
    output += operation("jal", label(function));
}

Jump::Jump(Block* block, llvm::BasicBlock* target) : Instruction(block)
{
    output += operation("j", label(target));
}

Store::Store(Block* block, llvm::Value* t1, llvm::Value* t2) : Instruction(block)
{
    const auto isWord = t1->getType()->getIntegerBitWidth() == 32;
    const auto index1 = mapper()->loadValue(output, t1);
    const auto index2 = mapper()->loadValue(output, t2);

    output += operation(isWord ? "sw" : "sb", reg(index1), reg(index2));
}

Store::Store(Block* block, llvm::Value* t1, llvm::GlobalVariable* variable) : Instruction(block)
{
    const auto isWord = t1->getType()->getIntegerBitWidth() == 32;
    const auto index1 = mapper()->loadValue(output, t1);

    output += operation(isWord ? "sw" : "sb", reg(index1), label(variable));
}

void Block::append(Instruction* instruction)
{
    instruction->setBlock(this);
    instructions.emplace_back(instruction);
}

void Block::appendBeforeLast(Instruction* instruction)
{
    instruction->setBlock(this);
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

void Block::setFunction(Function* func)
{
    function = func;
}


void Function::append(Block* block)
{
    block->setFunction(this);
    blocks.emplace_back(block);
}

void Function::print(std::ostream& os) const
{
    os << label(function) << ":\n";
    os << operation("addi", "$sp", "$sp", std::to_string(-mapper.getSize()));
    for(const auto& block : blocks)
    {
        block->print(os);
    }
    os << operation("addi", "$sp", "$sp", std::to_string(mapper.getSize()));
}

void Function::setModule(Module* mod)
{
    module = mod;
}

RegisterMapper* Function::getMapper()
{
    return &mapper;
}

Module* Function::getModule()
{
    return module;
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
    for(auto variable : floats)
    {
        os << label(variable) << ": .float " << variable->getValueAPF().convertToFloat() << '\n';
    }
    for(auto variable : globals)
    {
        // TODO:
    }

    os << ".text\n";
    for(const auto& function : functions)
    {
        function->print(os);
    }
}

void Module::addGlobal(llvm::GlobalVariable* variable)
{
    globals.emplace(variable);
}

void Module::addFloat(llvm::ConstantFP* variable)
{
    floats.emplace(variable);
}


} // namespace mips
