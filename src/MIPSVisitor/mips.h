//
// Created by ward on 5/7/20.
//

#ifndef COMPILER_MIPS_H
#define COMPILER_MIPS_H

#endif //COMPILER_MIPS_H

#include <vector>
#include <iostream>
#include <string>

namespace mips {
	class Instruction {
	public:
		virtual void print(std::ostream& os) const = 0;

		virtual ~Instruction() { }
	};

	class li : public Instruction {
	public:
		void print(std::ostream& os) const final
		{
			os << "li" << std::endl;
		}
	};

	class move : public Instruction {
	public:
		void print(std::ostream& os) const final
		{
			os << "move" << std::endl;
		}
	};

	class l : public Instruction {
	public:
		l(bool isCharacter)
				:isCharacter(isCharacter) { }

		void print(std::ostream& os) const final
		{
			os << 'l' << (isCharacter ? 'b' : 'w') << std::endl;
		}

	private:
		bool isCharacter;
	};

	class bin : public Instruction {
	public:
		bin(bool immediate, std::string type)
				:immediate(immediate), type(type) { };

		void print(std::ostream& os) const final
		{
			os << type << (immediate ? "i" : "") << 'u' << std::endl;
		}

	private:
		bool immediate;
		std::string type;
	};

	class s : public Instruction {
	public:
		s(bool isCharacter)
				:isCharacter(isCharacter) { }

		void print(std::ostream& os) const final
		{
			os << 's' << (isCharacter ? 'b' : 'w') << std::endl;
		}

	private:
		bool isCharacter;
	};

	class Block {
	public:
		void append(Instruction* instruction) { instructions.emplace_back(instruction); }

		void print(std::ostream& os) const
		{
			for (const auto& instruction : instructions) {
				instruction->print(os);
			}
		}

		virtual ~Block()
		{
			for (const auto& instruction : instructions) {
				delete instruction;
			}
		}

	private:
		std::vector<Instruction*> instructions;
	};

	class Function {
	public:
		void append(Block* block) { blocks.emplace_back(block); }

		void addToStack(const unsigned int size)
		{
			stackSize += size;
		}

		void print(std::ostream& os) const
		{
			os << stackSize << std::endl;
			for (const auto& block : blocks) {
				block->print(os);
			}
		}

		virtual ~Function()
		{
			for (const auto& block : blocks) {
				delete block;
			}
		}

	private:
		std::vector<Block*> blocks;
		unsigned int stackSize = 0;
	};

	class Module {
	public:
		void append(Function* function) { functions.emplace_back(function); }

		void print(std::ostream& os) const
		{
			for (const auto& function : functions) {
				function->print(os);
			}
		}

		virtual ~Module()
		{
			for (const auto& function : functions) {
				delete function;
			}
		}

	private:
		std::vector<Function*> functions;
	};
}