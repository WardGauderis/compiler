//
// Created by ward on 5/7/20.
//

#ifndef COMPILER_MIPS_H
#define COMPILER_MIPS_H

#endif //COMPILER_MIPS_H

#include <vector>

namespace mips {
	class Instruction {
	};

	class li : public Instruction {
	};

	class move : public Instruction {
	};

	class Block {
	public:
		void append(Instruction* instruction) { instructions.emplace_back(instruction); }

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

		virtual ~Function()
		{
			for (const auto& block : blocks) {
				delete block;
			}
		}

	private:
		std::vector<Block*> blocks;
	};

	class Module {
	public:
		void append(Function* function) { functions.emplace_back(function); }

		void print(const std::filesystem::path& output) { }

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