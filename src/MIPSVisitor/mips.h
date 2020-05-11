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

class RegisterMapper
{
    public:
    RegisterMapper() : emptyRegisters(), registerSize{25, 31}, nextSpill{0,0}, stackSize(0)
    {
        emptyRegisters[0].resize(26);
        std::iota(emptyRegisters[0].begin(), emptyRegisters[0].end(), 3);

        emptyRegisters[1].resize(32);
        std::iota(emptyRegisters[1].begin(), emptyRegisters[1].end(), 1);
    }

    uint loadValue(std::string& output, llvm::Value* id);

    void storeValue(std::string& output, llvm::Value* id);

    void cleanupRegisters(std::string& output);

    void storeRegisters(std::string& output);

    void loadRegisters(std::string& output);

    [[nodiscard]] uint getSize() const noexcept;

    private:
    std::array<std::vector<uint>, 2> emptyRegisters;
    std::array<std::vector<uint>, 2> usedRegisters;
    std::array<std::vector<uint>, 2> tempRegisters;

    std::array<std::map<llvm::Value*, uint>, 2> registerDescriptors;
    std::array<std::map<llvm::Value*, uint>, 2> addressDescriptors;

    std::array<uint, 2> registerSize;
    std::array<uint, 2> nextSpill;
    uint stackSize;
};

class Instruction
{
    public:
    Instruction() = default;

    Instruction(const std::string& type, llvm::Value* t1, llvm::Value* t2, bool isFloat);

    Instruction(const std::string& type, llvm::Value* t1, llvm::Value* t2, llvm::Value* t3, bool isFloat);

    void print(std::ostream& os);

    void setMapper(std::shared_ptr<RegisterMapper> imapper);

    protected:
    std::shared_ptr<RegisterMapper> mapper = nullptr;
    std::string output;
};

// move
struct Move : public Instruction
{
    Move(llvm::Value* t1, llvm::Value* t2);
};

struct Convert : public Instruction
{
    Convert(llvm::Value* t1, llvm::Value* t2);
};

// lw, li, lb, l.s
struct Load : public Instruction
{
    Load(llvm::Value* t1, llvm::Value* t2);
    Load(llvm::Value* t1, llvm::GlobalVariable* variable);
};

// add, sub, mul
struct Arithmetic : public Instruction
{
    Arithmetic(std::string type, llvm::Value* t1, llvm::Value* t2, llvm::Value* t3);
};

// modulo
struct Modulo : public Instruction
{
    Modulo(llvm::Value* t1, llvm::Value* t2, llvm::Value* t3);
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
    explicit Store(llvm::Value* t1, llvm::Value* t2);
    explicit Store(llvm::Value* t1, llvm::GlobalVariable* variable);
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

    void addGlobal(llvm::GlobalVariable* variable);

    private:
    std::vector<std::unique_ptr<Function>> functions;
    std::vector<llvm::GlobalVariable*> variables;
};


} // namespace mips