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
    RegisterMapper() : emptyRegisters(), stackSize(0)
    {
        emptyRegisters[0].resize(26);
        std::iota(emptyRegisters[0].begin(), emptyRegisters[0].end(), 2);

        emptyRegisters[1].resize(32);
        std::iota(emptyRegisters[1].begin(), emptyRegisters[1].end(), 0);
    }

    // gets the register for the value, if it is not yet in a register put it in one.
    uint getRegister(std::ostream& os, llvm::Value* id);

    void popValue(llvm::Value* id);

    private:
    std::array<std::vector<uint>, 2> emptyRegisters;
    std::array<std::map<llvm::Value*, uint>, 2> registerDescriptors;
    std::array<std::map<llvm::Value*, uint>, 2> addressDescriptors;
    uint stackSize;
};

class Instruction
{
    public:
    Instruction() : mapper(nullptr)
    {
    }

    virtual void print(std::ostream& os) const = 0;

    protected:
    std::shared_ptr<RegisterMapper> mapper;
};

class Move : public Instruction
{
    public:
    Move(llvm::Value* t1, llvm::Value* t2) : t1(t1), t2(t2)
    {
        assertSame(t1, t2);
    }

    void print(std::ostream& os) const final;

    private:
    llvm::Value* t1;
    llvm::Value* t2;
};

class Load : public Instruction
{
    public:
    Load(llvm::Value* t1, llvm::Value* t2, int offset)
    : t1(t1), t2(t2), ivalue(offset), fvalue(0), label(), immediate(false)
    {
    }

    Load(llvm::Value* t1, int value)
    : t1(t1), t2(nullptr), ivalue(value), fvalue(0), label(), immediate(true)
    {
    }

    Load(llvm::Value* t1, float value)
    : t1(t1), t2(nullptr), ivalue(0), fvalue(value), label(), immediate(true)
    {
    }

    Load(llvm::Value* t1, std::string label)
    : t1(t1), t2(nullptr), ivalue(0), fvalue(0), label(std::move(label)), immediate(false)
    {
    }

    void print(std::ostream& os) const final;

    private:
    llvm::Value* t1;
    llvm::Value* t2;

    int ivalue;
    float fvalue;

    std::string label;
    bool immediate;
};

class Arithmetic : public Instruction
{
    public:
    Arithmetic(std::string operation, llvm::Value* t1, llvm::Value* t2, llvm::Value* t3)
    : operation(std::move(operation)), t1(t1), t2(t2), t3(t3)
    {
        assertSame(t1, t2, t3);
    }

    Arithmetic(std::string operation, llvm::Value* t1, llvm::Value* t2, int immediate)
    : operation(std::move(operation)), t1(t1), t2(t2), t3(nullptr), immediate(immediate)
    {
        assertInt(t1);
        assertInt(t2);
    }

    void print(std::ostream& os) const final;

    private:
    std::string operation;
    llvm::Value* t1;
    llvm::Value* t2;
    llvm::Value* t3;

    int immediate;
};

class Modulo : public Instruction
{
    public:
    Modulo(llvm::Value* t1, llvm::Value* t2, llvm::Value* t3 = nullptr) : t1(t1), t2(t2), t3(t3)
    {
        assertSame(t1, t2, t3);
    }

    void print(std::ostream& os) const final;

    private:
    llvm::Value* t1;
    llvm::Value* t2;
    llvm::Value* t3;
};


class Comparison : public Instruction
{
    public:
    explicit Comparison(std::string operation, llvm::Value* t1, llvm::Value* t2, llvm::Value* t3)
    : operation(std::move(operation)), t1(t1), t2(t2), t3(t3)
    {
    }

    void print(std::ostream& os) const final;

    private:
    std::string operation;
    llvm::Value* t1;
    llvm::Value* t2;
    llvm::Value* t3;
};

// beq, bgtz, blez, bne, ...
class Branch : public Instruction
{
    public:
    explicit Branch(std::string operation, llvm::Value* t1, llvm::Value* t2, std::string label)
    : operation(std::move(operation)), t1(t1), t2(t2), label(std::move(label))
    {
    }

    void print(std::ostream& os) const final;

    private:
    std::string operation;
    llvm::Value* t1;
    llvm::Value* t2;
    std::string label;
};

class Jal : public Instruction
{
    public:
    explicit Jal(std::string label) : label(std::move(label)), link(link)
    {
    }

    void print(std::ostream& os);

    private:
    std::string label;
    bool link;
};

class Store : public Instruction
{
    public:
    explicit Store(bool isCharacter) : isCharacter(isCharacter)
    {
    }

    void print(std::ostream& os) const final;

    private:
    bool isCharacter;
};

class Block
{
    friend class Function;

    public:
    Block() : mapper(nullptr)
    {
    }

    void append(Instruction* instruction);

    void print(std::ostream& os) const;

    private:
    std::string label;
    std::vector<std::unique_ptr<Instruction>> instructions;
    std::shared_ptr<RegisterMapper> mapper;
};

class Function
{
    public:
    Function() : mapper(new RegisterMapper)
    {
    }

    void append(Block* block);

    void print(std::ostream& os) const;

    private:
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