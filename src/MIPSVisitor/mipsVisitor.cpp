//
// Created by ward on 5/7/20.
//

#include "mipsVisitor.h"
#include <iostream>

using namespace llvm;
using namespace mips;

void MIPSVisitor::convertIR(llvm::Module& module)
{
	visit(module);
}

void MIPSVisitor::print(const std::filesystem::path& output)
{
	module.print(output);
}

[[maybe_unused]] void MIPSVisitor::visitModule(llvm::Module& M)
{

}

void MIPSVisitor::visitFunction(llvm::Function& F)
{
	currentFunction = new mips::Function();
	module.append(currentFunction);
}

void MIPSVisitor::visitBasicBlock(BasicBlock& BB)
{
	currentBlock = new mips::Block();
	currentFunction->append(currentBlock);
}

[[maybe_unused]] void MIPSVisitor::visitReturnInst(ReturnInst& I)
{
	if (auto i = dyn_cast<Constant>(I.getReturnValue())) {
		std::cout << "ok";
		currentBlock->append(new li());
	}
	else {
		currentBlock->append(new move());
	}
}
