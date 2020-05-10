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
std::string reg(uint num)
{
    return "$" + std::to_string(num);
}

void operation_impl(std::ostream& os)
{

}

template<typename... Args>
std::string operation(std::ostream& os, const std::string& operation, const Args&... args)
{
    os << operation << ' ';

    std::make_index_sequence<sizeof...(Args) - 1>{};

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
    uint getRegister(std::ostream& os, llvm::Value* id)
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

    void popValue(llvm::Value* id)
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

    void print(std::ostream& os) const final
    {
        const auto index1 = mapper->getRegister(os, t1);
        const auto index2 = mapper->getRegister(os, t2);

        os << (isFloat(t1) ? "mov.s " : "move ");
        os << reg(index1) << ',' << reg(index2) << '\n';
    }

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

    void print(std::ostream& os) const final
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

    void print(std::ostream& os) const final
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
            os << operation << "iu " << reg(index1) + ',' + reg(index2) + ','
               << std::to_string(immediate) << '\n';
        }
        else
        {
            const auto index3 = mapper->getRegister(os, t3);
            os << operation << "u " << reg(index1) + ',' + reg(index2) + ',' << reg(index3) << '\n';
        }
    }

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

    void print(std::ostream& os) const final
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

    private:
    llvm::Value* t1;
    llvm::Value* t2;
    llvm::Value* t3;
};

// beq, bgtz, blez, bne, ...
class Comparison : public Instruction
{
    public:
    explicit Comparison(std::string operation, llvm::Value* t1, llvm::Value* t2, llvm::Value* t3)
    : operation(std::move(operation)), t1(t1), t2(t2), t3(t3)
    {
    }

    void print(std::ostream& os) const final
    {
        const auto index1 = mapper->getRegister(os, t1);
        const auto index2 = mapper->getRegister(os, t2);
        const auto index3 = mapper->getRegister(os, t3);

        os << operation << ' ' << reg(index1) << ',' << reg(index2) << ',' << reg(index3) << '\n';
    }

    private:
    std::string operation;
    llvm::Value* t1;
    llvm::Value* t2;
    llvm::Value* t3;
};

class Branch : public Instruction
{
    public:
    explicit Branch(std::string operation, llvm::Value* t1, llvm::Value* t2, std::string label)
    : operation(std::move(operation)), t1(t1), t2(t2), label(std::move(label))
    {
    }

    void print(std::ostream& os) const final
    {
        const auto index1 = mapper->getRegister(os, t1);
        const auto index2 = mapper->getRegister(os, t2);

        os << operation << ' ' << reg(index1) << ',' << reg(index2) << ',' << label << '\n';
    }

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
    friend class Function;

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
        os << label << ":\n";
        for(const auto& instruction : instructions)
        {
            instruction->print(os);
        }
    }

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

    void append(Block* block)
    {
        block->mapper = mapper;
        blocks.emplace_back(block);
    }

    void print(std::ostream& os) const
    {
        for(const auto& block : blocks)
        {
            block->print(os);
        }
    }

    private:
    std::vector<std::unique_ptr<Block>> blocks;
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

    private:
    std::vector<std::unique_ptr<Function>> functions;
};


} // namespace mips