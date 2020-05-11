//============================================================================
// @author      : Thomas Dooms
// @date        : 5/10/20
// @copyright   : BA2 Informatica - Thomas Dooms - University of Antwerp
//============================================================================

#pragma once

#include <iostream>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Value.h>
#include <map>
#include <numeric>
#include <queue>
#include <set>
#include <string>
#include <vector>

namespace mips
{

class Block;
class Function;
class Module;

class RegisterMapper
{
    public:
    explicit RegisterMapper(Module* module)
    : emptyRegisters(), module(module), registerSize{ 25, 31 }, nextSpill{ 0, 0 }
    {
        emptyRegisters[0].resize(26);
        std::iota(emptyRegisters[0].begin(), emptyRegisters[0].end(), 4);

        emptyRegisters[1].resize(32);
        std::iota(emptyRegisters[1].begin(), emptyRegisters[1].end(), 1);
    }

    uint loadValue(std::string& output, llvm::Value* id);

    uint getTemp();

    void storeValue(std::string& output, llvm::Value* id);

    void storeRegisters(std::string& output);

    [[nodiscard]] uint getSize() const noexcept;

    private:
    std::array<std::vector<uint>, 2> emptyRegisters;
    Module* module;

    std::array<std::map<llvm::Value*, uint>, 2> registerDescriptors;
    std::array<std::map<llvm::Value*, uint>, 2> addressDescriptors;

    std::array<uint, 2> registerSize;
    std::array<uint, 2> nextSpill;

    int tempReg = 0;
    uint stackSize = 0;
};

class Instruction
{
    public:
    explicit Instruction(Block* block) : block(block)
    {
    }

    void print(std::ostream& os);

    void setBlock(Block* b);

    RegisterMapper* mapper();
    Module* module();

    protected:
    Block* block;
    std::string output;
};

// move
struct Move : public Instruction
{
    Move(Block* block, llvm::Value* t1, llvm::Value* t2);
};

struct Convert : public Instruction
{
    Convert(Block* block, llvm::Value* t1, llvm::Value* t2);
};

// lw, li, lb, l.s
struct Load : public Instruction
{
    Load(Block* block, llvm::Value* t1, llvm::Value* t2);
    Load(Block* block, llvm::Value* t1, llvm::GlobalVariable* variable);
};

// add, sub, mul
struct Arithmetic : public Instruction
{
    Arithmetic(Block* block, std::string type, llvm::Value* t1, llvm::Value* t2, llvm::Value* t3);
};

// modulo
struct Modulo : public Instruction
{
    Modulo(Block* block, llvm::Value* t1, llvm::Value* t2, llvm::Value* t3);
};

struct NotEquals : public Instruction
{
    NotEquals(Block* block, llvm::Value* t1, llvm::Value* t2, llvm::Value* t3);
};

struct Branch : public Instruction
{
    explicit Branch(Block* block, llvm::Value* t1, llvm::BasicBlock* target, bool eqZero);
};

// jal
struct Call : public Instruction
{
    explicit Call(Block* block, llvm::Function* function);
};

// j
struct Jump : public Instruction
{
    explicit Jump(Block* block, llvm::BasicBlock* target);
};

// sw, sb
struct Store : public Instruction
{
    explicit Store(Block* block, llvm::Value* t1, llvm::Value* t2);
    explicit Store(Block* block, llvm::Value* t1, llvm::GlobalVariable* variable);
};

class Block
{
    friend class Instruction;

    public:
    explicit Block(Function* function, llvm::BasicBlock* block) : block(block), function(function)
    {
    }

    void append(Instruction* instruction);

    void appendBeforeLast(Instruction* instruction);

    void print(std::ostream& os) const;

    llvm::BasicBlock* getBlock();

    void setFunction(Function* func);

    private:
    llvm::BasicBlock* block;
    std::vector<std::unique_ptr<Instruction>> instructions;
    Function* function;
};

class Function
{
    friend class Block;

    public:
    explicit Function(Module* module, llvm::Function* function) : function(function), mapper(module), module(module)
    {
    }

    void append(Block* block);

    void print(std::ostream& os) const;

    void setModule(Module* mod);

    RegisterMapper* getMapper();

    Module* getModule();

    Block* getBlockByBasicBlock(llvm::BasicBlock* block);

    private:
    llvm::Function* function;
    std::vector<std::unique_ptr<Block>> blocks;

    RegisterMapper mapper;
    Module* module;
};

class Module
{
    public:
    void append(Function* function);

    void print(std::ostream& os) const;

    void addGlobal(llvm::GlobalVariable* variable);

    void addFloat(llvm::ConstantFP* variable);

    private:
    std::vector<std::unique_ptr<Function>> functions;
    std::set<llvm::GlobalVariable*> globals;
    std::set<llvm::ConstantFP*> floats;
};


} // namespace mips