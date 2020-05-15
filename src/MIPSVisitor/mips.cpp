//============================================================================
// @author      : Thomas Dooms
// @date        : 5/10/20
// @copyright   : BA2 Informatica - Thomas Dooms - University of Antwerp
//============================================================================

#include "mips.h"
#include "../errors.h"
#include <llvm/IR/Constants.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/Type.h>

namespace
{
std::string reg(uint num)
{
    return (num >= 32 ? "$f" : "$") + std::to_string(num % 32);
}

template <typename Ptr>
std::string label(Ptr* ptr)
{
    return "g" + std::to_string(reinterpret_cast<size_t>(ptr)); // nobody can see this line
}

std::string label(llvm::Function* ptr)
{
    return ptr->getName();
}

std::string operation(std::string&& operation, std::string&& t1 = "", std::string&& t2 = "", std::string&& t3 = "")
{
    std::string res = (operation + ' ');
    if(not t1.empty()) res += t1 + ",";
    if(not t2.empty()) res += t2 + ",";
    if(not t3.empty()) res += t3 + ",";
    res.back() = '\n';
    return res;
}

std::string move(uint to, uint from, bool fl)
{
    if(to == from) return "";
    return operation(fl ? "mov.s" : "move", reg(to), reg(from));
}

bool isFloat(llvm::Value* value)
{
    return value->getType()->isFloatTy();
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

RegisterMapper::RegisterMapper(Module* module, llvm::Function* function)
: module(module), function(function)
{
    emptyRegisters[0].resize(end[0] - start[0]);
    std::iota(emptyRegisters[0].begin(), emptyRegisters[0].end(), start[0]);

    emptyRegisters[1].resize(end[1] - start[1]);
    std::iota(emptyRegisters[1].begin(), emptyRegisters[1].end(), start[1]);

    savedRegisters[0] = std::vector<uint>(32, std::numeric_limits<uint>::max());
    savedRegisters[1] = std::vector<uint>(32, std::numeric_limits<uint>::max());

    registerValues[0] = std::vector<llvm::Value*>(32, nullptr);
    registerValues[1] = std::vector<llvm::Value*>(32, nullptr);

    for(auto& arg : function->args())
    {
        const auto fl = isFloat(&arg);
        argsSize += 4;
        addressDescriptors[fl].emplace(&arg, function->arg_size() * 4 - argsSize);
    }
}

uint RegisterMapper::loadValue(std::string& output, llvm::Value* id)
{
    const auto fl = isFloat(id);
    const auto result = [&](auto r) { return r + 32 * fl; };

    // try to place constant value into temp register and be done with it
    const auto tmp = getTempRegister(fl);
    if(placeConstant(output, tmp, id))
    {
        return tmp;
    }

    // we try to find if it is stored in a register already
    if(const auto iter = registerDescriptors[fl].find(id); iter == registerDescriptors[fl].end())
    {
        // we find a suitable register, either by finding an empty one or spilling another one
        auto index = -1;
        if(emptyRegisters[fl].empty())
        {
            index = getNextSpill(fl);
        }
        else
        {
            index = emptyRegisters[fl].back();
            emptyRegisters[fl].pop_back();

            savedRegisters[fl][index] = argsSize + saveSize;
            stores += operation(fl ? "swc1" : "sw", reg(result(index)), std::to_string(argsSize + saveSize) + "($sp)");
            saveSize += 4;
        }

        // spill if nescessary
        storeRegister(output, index, fl);

        // place it in the desired register
        placeValue(output, index, id);

        return result(index);
    }
    else
    {
        return result(iter->second);
    }
}

void RegisterMapper::loadSaved(std::string& output)
{
    for(size_t i = 0; i < savedRegisters[0].size(); i++)
    {
        if(savedRegisters[0][i] == std::numeric_limits<uint>::max()) continue;
        output += operation("lw", reg(i), std::to_string(savedRegisters[0][i]) + "($sp)");
    }

    for(size_t i = 0; i < savedRegisters[1].size(); i++)
    {
        if(savedRegisters[1][i] == std::numeric_limits<uint>::max()) continue;
        output += operation("lwc1", reg(i + 32), std::to_string(savedRegisters[1][i]) + "($sp)");
    }
}

void RegisterMapper::loadReturnValue(std::string& output, llvm::Value* id)
{
    const auto fl = isFloat(id);
    const auto index1 = loadValue(output, id);
    output += move(index1, fl ? 32 : 2, fl);
}

bool RegisterMapper::placeConstant(std::string& output, uint index, llvm::Value* id)
{
    if(const auto& constant = llvm::dyn_cast<llvm::GlobalVariable>(id))
    {
        output += operation("la", reg(index), label(id));
        return true;
    }
    else if(const auto& constant = llvm::dyn_cast<llvm::ConstantInt>(id))
    {
        const auto immediate = int(constant->getSExtValue());
        output += operation("li", reg(index), std::to_string(immediate));
        return true;
    }
    else if(const auto& constant = llvm::dyn_cast<llvm::ConstantFP>(id))
    {
        module->addFloat(constant);
        output += operation("l.s", reg(index + 32), label(id));
        return true;
    }

    const auto fl = isFloat(id);
    const auto address = pointerDescriptors[fl].find(id);
    if(address != pointerDescriptors[fl].end())
    {
        output += operation("la", reg(index), std::to_string(address->second) + "($sp)");
        return true;
    }

    return false;
}

bool RegisterMapper::placeValue(std::string& output, uint index, llvm::Value* id)
{
    const auto fl = isFloat(id);
    const auto addrIter = addressDescriptors[fl].find(id);

    // if it is already stored on the stack, we put that value in the register
    if(addrIter != addressDescriptors[fl].end())
    {
        if(fl)
        {
            const auto tempReg = getTempRegister(false);
            output += operation("lw", reg(tempReg), std::to_string(addrIter->second) + "($sp)");
            output += operation("mtc1", reg(tempReg), reg(index + 32));
        }
        else
        {
            output += operation("lw", reg(index), std::to_string(addrIter->second) + "($sp)");
        }

        addressDescriptors[fl].erase(addrIter);
        registerDescriptors[fl].emplace(id, index);
        return true;
    }

    registerDescriptors[fl].emplace(id, index);
    return false;
}

uint RegisterMapper::getTempRegister(bool fl)
{
    if(not(temp[fl] == 0 or temp[fl] == 1))
    {
        throw InternalError("integer temp register has wrong value for some reason");
    }
    const auto tmp = temp[fl];
    temp[fl] = !temp[fl];
    return (fl ? 32 : 2) + tmp;
}

uint RegisterMapper::getNextSpill(bool fl)
{
    spill[fl]++;
    if(spill[fl] > end[fl])
    {
        spill[fl] = start[fl];
    }
    return spill[fl];
}

void RegisterMapper::storeValue(std::string& output, llvm::Value* id)
{
    const auto fl = isFloat(id);
    const auto iter = registerDescriptors[fl].find(id);

    if(iter != registerDescriptors[fl].end())
    {
        // spills the value
        output += operation("sw", reg(iter->second), std::to_string(argsSize + saveSize) + "($sp)");
        saveSize += 4;

        emptyRegisters[fl].push_back(iter->second);
        registerDescriptors[fl].erase(iter);
        addressDescriptors[fl].emplace(id, iter->second);
    }
}

void RegisterMapper::storeRegister(std::string& output, uint index, bool fl)
{
    if(registerValues[fl][index] != nullptr)
    {
        storeValue(output, registerValues[fl][index]);
    }
}

void RegisterMapper::storeReturnValue(std::string& output, llvm::Value* id)
{
    const auto fl = isFloat(id);
    const auto index1 = loadValue(output, id);
    output += move(fl ? 32 : 2, index1, fl);
}

void RegisterMapper::allocateValue(std::string& output, llvm::Value* id, llvm::Type* type)
{
    const auto fl = isFloat(id);
    pointerDescriptors[fl].emplace(id, argsSize + saveSize);
    saveSize += module->layout.getTypeStoreSize(type);
}

int RegisterMapper::getSaveSize() const noexcept
{
    return saveSize;
}

int RegisterMapper::getArgsSize() const noexcept
{
    return argsSize;
}

void RegisterMapper::print(std::ostream& os) const
{
    os << stores;
}

void Instruction::print(std::ostream& os)
{
    os << output;
}

RegisterMapper* Instruction::mapper()
{
    return block->function->getMapper();
}

Module* Instruction::module()
{
    return block->function->module;
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

        output += operation("cvt.w.s", reg(index2), reg(index2));
        output += operation("mfc1", reg(index1), reg(index2));
    }
    else
    {
        // int to float
        assertFloat(t1);

        output += operation("mtc1", reg(index2), reg(index1));
        output += operation("cvt.s.w", reg(index1), reg(index1));
    }
}

Load::Load(Block* block, llvm::Value* t1, llvm::Value* t2) : Instruction(block)
{
    const auto index1 = mapper()->loadValue(output, t1);
    const auto index2 = mapper()->loadValue(output, t2);

    if(isFloat(t1))
    {
        output += operation("lwc1", reg(index1), '(' + reg(index2) + ')');
    }
    else
    {
        const bool isWord = module()->layout.getTypeStoreSize(t1->getType()) == 4;
        output += operation(isWord ? "lw" : "lb", reg(index1), "(" + reg(index2) + ")");
    }
}

Arithmetic::Arithmetic(Block* block, std::string type, llvm::Value* t1, llvm::Value* t2, llvm::Value* t3)
: Instruction(block)
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

NotEquals::NotEquals(Block* block, llvm::Value* t1, llvm::Value* t2, llvm::Value* t3)
: Instruction(block)
{
    const auto index1 = mapper()->loadValue(output, t1);
    const auto index2 = mapper()->loadValue(output, t2);
    const auto index3 = mapper()->loadValue(output, t3);

    output += operation("c.eq.s", reg(index1), reg(index2), reg(index3));
    output += operation("cmp", reg(index1), reg(index1), reg(0));
}

Branch::Branch(Block* block, llvm::Value* t1, llvm::BasicBlock* target, bool eqZero)
: Instruction(block)
{
    const auto index1 = mapper()->loadValue(output, t1);

    output += operation(eqZero ? "beqz" : "bnez", reg(index1), label(target));
}

Call::Call(Block* block, llvm::Function* function, std::vector<llvm::Value*>&& arguments, llvm::Value* ret)
: Instruction(block), function(function), arguments(std::move(arguments)), ret(ret)
{
}

void Call::print(std::ostream& os)
{
    const int other = module()->getFunctionSize(function);
    auto iter = 4;

    //store parameters
    for(auto arg : arguments)
    {
        const auto index = mapper()->loadValue(output, arg);
        iter += 4;
        output += operation("sw", reg(index), std::to_string(-other - iter) + "($sp)");
    }

    const auto incr = module()->isStdio(function) ? 4 : other + iter;

    output += operation("sw", "$ra", "-4($sp)");
    output += operation("addi", "$sp", "$sp", std::to_string(-incr));
    output += operation("jal", label(function));
    output += operation("addi", "$sp", "$sp", std::to_string(incr));
    output += operation("lw", "$ra", "-4($sp)");

    if(ret != nullptr)
    {
        mapper()->loadReturnValue(output, ret);
    }
    os << output;
}

Return::Return(Block* block, llvm::Value* value) : Instruction(block)
{
    if(value != nullptr)
    {
        mapper()->storeReturnValue(output, value);
        mapper()->loadSaved(output);
    }

    output += operation("jr", "$ra");
}

Jump::Jump(Block* block, llvm::BasicBlock* target) : Instruction(block)
{
    output += operation("j", label(target));
}

Allocate::Allocate(Block* block, llvm::Value* t1, llvm::Type* type) : Instruction(block)
{
    mapper()->allocateValue(output, t1, type);
}

Empty::Empty(Block* block, llvm::Value* t1, llvm::Value* t2) : Instruction(block)
{
    const auto index1 = mapper()->loadValue(output, t1);
    mapper()->placeConstant(output, index1, t2);
}

Store::Store(Block* block, llvm::Value* t1, llvm::Value* t2) : Instruction(block)
{
    const auto index1 = mapper()->loadValue(output, t1);
    const auto index2 = mapper()->loadValue(output, t2);

    if(isFloat(t1))
    {
        output += operation("s.s", reg(index1), reg(index2));
    }
    else
    {
        const auto isWord = module()->layout.getTypeStoreSize(t1->getType()) == 4;
        output += operation(isWord ? "sw" : "sb", reg(index1), "(" + reg(index2) + ")");
    }
}

void Block::append(Instruction* instruction)
{
    instructions.emplace_back(instruction);
}

void Block::appendBeforeLast(Instruction* instruction)
{
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

void Function::append(Block* block)
{
    blocks.emplace_back(block);
}

void Function::print(std::ostream& os) const
{
    os << label(function) << ":\n";
    mapper.print(os);
    for(const auto& block : blocks)
    {
        block->print(os);
    }
}

bool Function::isMain() const
{
    return function->getName() == "main";
}


RegisterMapper* Function::getMapper()
{
    return &mapper;
}

llvm::Function* Function::getFunction()
{
    return function;
}

Block* Function::getBlockByBasicBlock(llvm::BasicBlock* block)
{
    const auto pred = [&](const auto& ptr) { return ptr->getBlock() == block; };
    const auto iter = std::find_if(blocks.begin(), blocks.end(), pred);
    return (iter == blocks.end()) ? nullptr : iter->get();
}

void Module::append(Function* function)
{
    if(function->isMain())
    {
        main = function;
    }
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
        os << ".align 2\n";
        if(variable->getValueType()->isIntegerTy() or variable->getValueType()->isPointerTy())
        {
            os << label(variable) << ": .word ";
            if(const auto* tmp = llvm::dyn_cast<llvm::ConstantInt>(variable->getInitializer()))
            {
                os << tmp->getSExtValue() << '\n';
            }
        }
        else if(variable->getValueType()->isFloatTy())
        {
            os << label(variable) << ": .float ";
            if(const auto* tmp = llvm::dyn_cast<llvm::ConstantFP>(variable->getInitializer()))
            {
                os << tmp->getValueAPF().convertToFloat() << '\n';
            }
        }
        else if(variable->getValueType()->isArrayTy()
                and variable->getValueType()->getContainedType(0)->isIntegerTy(8)
                and variable->hasInitializer())
        {
            os << label(variable) << ": .asciiz ";
            if(const auto* tmp = llvm::dyn_cast<llvm::ConstantDataArray>(variable->getInitializer()))
            {
                os << '"' << tmp->getAsString().data() << "\"\n";
            }
            else
            {
                throw InternalError("problem with llvm strings");
            }
        }
        else
        {
            os << label(variable) << ": .space ";
            os << layout.getTypeStoreSize(variable->getValueType());
            os << "\n";
        }
    }

    os << ".text\n";
    os << "$begin:\n";
    if(main)
    {
        const auto size = -static_cast<int>(main->getMapper()->getSaveSize());
        os << "addi $sp, $sp, " << size << '\n';
        os << "jal main\n";
        os << "move $4, $2\n";
    }
    else
    {
        os << "li $4, 0\n";
    }
    os << "li $2, 17\n";
    os << "syscall\n";

    if(printfIncluded)
    {
        std::ifstream file("asm/stdio.asm");
        if(not file.good())
        {
            throw std::runtime_error("could not read file stdio.asm");
        }
        std::string contents;

        file.seekg(0, std::ios::end);
        contents.reserve(file.tellg());
        file.seekg(0, std::ios::beg);

        contents.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

        os << contents;
    }

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

int Module::getFunctionSize(llvm::Function *function)
{
    if(function == printf or function == scanf)
    {
        return 0;
    }

    const auto pred = [&](const auto& ptr)
    {
        return ptr->getFunction() == function;
    };
    const auto iter = std::find_if(functions.begin(), functions.end(), pred);
    if(iter == functions.end())
    {
        throw std::logic_error("could not find given function");
    }
    return (*iter)->getMapper()->getSaveSize();
}

bool Module::isStdio(llvm::Function* function) const
{
    return function == printf or function == scanf;
}

void Module::includeStdio(llvm::Function* printf, llvm::Function* scanf)
{
    printfIncluded = true;
    this->printf = printf;
    this->scanf = scanf;
}


} // namespace mips
