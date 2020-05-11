//============================================================================
// @author      : Thomas Dooms
// @date        : 5/10/20
// @copyright   : BA2 Informatica - Thomas Dooms - University of Antwerp
//============================================================================

#pragma once

#include <iostream>
#include <llvm/IR/Value.h>
#include <numeric>
#include <string>
#include <vector>
#include <map>
#include <queue>

namespace mips
{

bool isFloat(llvm::Value* value);

void assertSame(llvm::Value* val1, llvm::Value* val2);

void assertSame(llvm::Value* val1, llvm::Value* val2, llvm::Value* t3);

void assertInt(llvm::Value* value);

void assertFloat(llvm::Value* value);

class RegisterMapper
{
    public:
    RegisterMapper() : emptyRegisters(), registerSize{26, 32}, nextSpill{0,0}, stackSize(0)
    {
        emptyRegisters[0].resize(26);
        std::iota(emptyRegisters[0].begin(), emptyRegisters[0].end(), 2);

        emptyRegisters[1].resize(32);
        std::iota(emptyRegisters[1].begin(), emptyRegisters[1].end(), 0);
    }

    uint loadValue(std::string& output, llvm::Value* id);

    void storeValue(std::string& output, llvm::Value* id);

    uint getSize() const noexcept;

    private:
    std::array<std::vector<uint>, 2> emptyRegisters;
    std::array<std::map<llvm::Value*, uint>, 2> registerDescriptors;
    std::array<std::map<llvm::Value*, uint>, 2> addressDescriptors;

    std::array<uint, 2> registerSize;
    std::array<uint, 2> nextSpill;
    uint stackSize;
};

class Instruction
{
    public:
    Instruction() : mapper(nullptr)
    {
    }

    void print(std::ostream& os);

    void setMapper(std::shared_ptr<RegisterMapper> imapper);

    protected:
    std::shared_ptr<RegisterMapper> mapper;
    std::string output;
};

// move
struct Move : public Instruction
{
    Move(llvm::Value* t1, llvm::Value* t2);
};

// lw, li
struct Load : public Instruction
{
    Load(llvm::Value* t1, llvm::Value* t2, int offset = 0);
    Load(llvm::Value* t1, int value);
    Load(llvm::Value* t1, float value);
    Load(llvm::Value* t1, llvm::GlobalVariable* variable);
};

// add, sub, mul
struct Arithmetic : public Instruction
{
    Arithmetic(std::string type, llvm::Value* t1, llvm::Value* t2, llvm::Value* t3);
    Arithmetic(std::string type, llvm::Value* t1, llvm::Value* t2, int immediate);
};

// modulo
struct Modulo : public Instruction
{
    Modulo(llvm::Value* t1, llvm::Value* t2, llvm::Value* t3);
};

// slt, sgt, slte
struct Comparison : public Instruction
{
    Comparison(std::string type, llvm::Value* t1, llvm::Value* t2, llvm::Value* t3);
};

struct NotEquals : public Instruction
{
    NotEquals(llvm::Value* t1, llvm::Value* t2, llvm::Value* t3);
};

// beq, bgtz, blez, bne, ...
struct Branch : public Instruction
{
    explicit Branch(std::string type, llvm::Value* t1, llvm::Value* t2, llvm::BasicBlock* block);
};

// jal
struct Call : public Instruction
{
    explicit Call(llvm::Function* function);
};

// j
struct Jump : public Instruction
{
    explicit Jump(llvm::BasicBlock* block);
};

// sw, sb
struct Store : public Instruction
{
    explicit Store(llvm::Value* t1, llvm::Value* t2, uint offset = 0);
    explicit Store(llvm::Value* t1, std::string label, uint offset = 0);
};

class Block
{
    public:
    explicit Block(llvm::BasicBlock* block) : block(block), mapper(nullptr)
    {
    }

    void append(Instruction* instruction);

    void appendBeforeLast(Instruction* instruction);

    void print(std::ostream& os) const;

    llvm::BasicBlock* getBlock();

    void setMapper(std::shared_ptr<RegisterMapper> imapper);

    private:
    llvm::BasicBlock* block;
    std::vector<std::unique_ptr<Instruction>> instructions;
    std::shared_ptr<RegisterMapper> mapper;
};

class Function
{
    public:
    explicit Function(llvm::Function* function) : function(function), mapper(new RegisterMapper)
    {
    }

    void append(Block* block);

    void print(std::ostream& os) const;

    Block* getBlockByBasicBlock(llvm::BasicBlock* block);

    private:
    llvm::Function* function;
    std::vector<std::unique_ptr<Block>> blocks;
    std::shared_ptr<RegisterMapper> mapper;
};

class Module
{
    public:
    void append(Function* function);

    void print(std::ostream& os) const;

    private:
    std::vector<std::unique_ptr<Function>> functions;
};


} // namespace mips