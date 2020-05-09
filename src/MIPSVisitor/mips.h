//
// Created by ward on 5/7/20.
//

#ifndef COMPILER_MIPS_H
#define COMPILER_MIPS_H

#endif // COMPILER_MIPS_H

#include <iostream>
#include <numeric>
#include <string>
#include <vector>

namespace mips
{
class RegisterMapper
{
    public:
    RegisterMapper() : emptyRegisters{}
    {
        emptyRegisters[0].resize(26);
        std::iota(emptyRegisters[0].begin(), emptyRegisters[0].end(), 2);

        emptyRegisters[1].resize(26);
        std::iota(emptyRegisters[1].begin(), emptyRegisters[1].end(), 2);
    }

    // gets the register for the value, if it is not yet in a register put it in one.
    uint getInternal(llvm::Value* value)
    {
        const bool isFloat = value->getType()->isFloatTy();
        const auto iter = registerDescriptors[isFloat].find(value);

        if(iter == registerDescriptors[isFloat].end())
        {
            if(emptyRegisters[isFloat].empty())
            {
                // aiaiai
                return -1;
            }
            else
            {
                const auto res = emptyRegisters[isFloat].back();
                emptyRegisters[isFloat].pop_back();
                return res;
            }
        }
        else
        {
            return iter->second;
        }
    }

    void popInternal(llvm::Value* value)
    {
        const bool isFloat = value->getType()->isFloatTy();
        const auto iter = registerDescriptors[isFloat].find(value);

        if(iter != registerDescriptors[isFloat].end())
        {
            emptyRegisters[isFloat].push_back(iter->second);
            registerDescriptors[isFloat].erase(iter);
        }
        addressDescriptors[isFloat].erase(value);
    }

    private:
    std::array<std::vector<uint>, 2> emptyRegisters;
    std::array<std::map<llvm::Value*, uint>, 2> registerDescriptors;
    std::array<std::map<llvm::Value*, uint>, 2> addressDescriptors;
};

class Instruction
{
    public:
    virtual void print(std::ostream& os) const = 0;

    virtual ~Instruction() = default;
};

class Li : public Instruction
{
    public:
    void print(std::ostream& os) const final
    {
        os << "li" << std::endl;
        os << "ori" << '\n';
    }
};

class Move : public Instruction
{
    public:
    void print(std::ostream& os) const final
    {
        os << "move" << std::endl;
    }
};

class Load : public Instruction
{
    public:
    explicit Load(bool isCharacter) : isCharacter(isCharacter)
    {
    }

    void print(std::ostream& os) const final
    {
        os << 'l' << (isCharacter ? 'b' : 'w') << std::endl;
    }

    private:
    bool isCharacter;
};

class Arithmetic : public Instruction
{
    public:
    Arithmetic(bool immediate, std::string operation)
    : immediate(immediate), operation(std::move(operation))
    {
    }

    void print(std::ostream& os) const final
    {
        os << operation << (immediate ? "i" : "") << 'u' << std::endl;
    }

    private:
    bool immediate;
    std::string operation;
};

class Multiplication : public Instruction
{
    public:
    explicit Multiplication() = default;

    void print(std::ostream& os) const final
    {
        os << "mulu" << std::endl;
        os << "mflo" << std::endl;
    }
};

class Division : public Instruction
{
    public:
    explicit Division(bool modulo) : modulo(modulo)
    {
    }

    void print(std::ostream& os) const final
    {
        os << "divu" << std::endl;
        if(modulo) os << "mflo";
        else os << "mfhi";
    }

    private:
    bool modulo;
};

class Comparison : public Instruction
{
    public:
    explicit Comparison(std::string operation, llvm::Value* ) : operation(std::move(operation))
    {
    }

    explicit Comparison(std::string operation, ) : operation(std::move(operation))
    {
    }

    void print(std::ostream& os) const final
    {
        os << operation << 'u' << std::endl;
    }

    private:
    std::string operation;
};

class Branch : public Instruction
{
    public:
    explicit Branch(std::string operation, std::string label)
    : operation(std::move(operation)), label(std::move(label))
    {
    }

    void print(std::ostream& os)
    {
        os << /* registers */ label << '\n';
    }

    private:
    std::string operation;
    std::string label;
};

class Jump : public Instruction
{
    public:
    explicit Jump(std::string label, bool link) : label(std::move(label)), link(link)
    {
    }

    void print(std::ostream& os)
    {
        os << (link ? "jal" : "j") << /* registers */ label << '\n';
    }

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

    void print(std::ostream& os) const final
    {
        os << 's' << (isCharacter ? 'b' : 'w') << std::endl;
    }

    private:
    bool isCharacter;
};

class Block
{
    public:
    Block() : mapper(nullptr)
    {
    }

    void append(Instruction* instruction)
    {
        instructions.emplace_back(instruction);
    }

    void print(std::ostream& os) const
    {
        for(const auto& instruction : instructions)
        {
            instruction->print(os);
        }
    }

    virtual ~Block()
    {
        for(const auto& instruction : instructions)
        {
            delete instruction;
        }
    }

    private:
    std::vector<Instruction*> instructions;
    std::shared_ptr<RegisterMapper> mapper;
};

class Function
{
    public:
    void append(Block* block)
    {
        blocks.emplace_back(block);
    }

    void addToStack(const unsigned int size)
    {
        stackSize += size;
    }

    void print(std::ostream& os) const
    {
        os << stackSize << std::endl;
        for(const auto& block : blocks)
        {
            block->print(os);
        }
    }

    virtual ~Function()
    {
        for(const auto& block : blocks)
        {
            delete block;
        }
    }

    private:
    std::vector<Block*> blocks;
    uint stackSize = 0;

    std::shared_ptr<RegisterMapper> mapper;
};

class Module
{
    public:
    void append(Function* function)
    {
        functions.emplace_back(function);
    }

    void print(std::ostream& os) const
    {
        for(const auto& function : functions)
        {
            function->print(os);
        }
    }

    virtual ~Module()
    {
        for(const auto& function : functions)
        {
            delete function;
        }
    }

    private:
    std::vector<Function*> functions;
};


} // namespace mips