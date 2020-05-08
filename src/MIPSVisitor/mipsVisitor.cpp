//
// Created by ward on 5/7/20.
//

#include "mipsVisitor.h"
#include <fstream>

using namespace llvm;
using namespace mips;

MIPSVisitor::MIPSVisitor(const llvm::Module& module)
		:layout(module.getDataLayout()) { }

void MIPSVisitor::convertIR(llvm::Module& module)
{
	visit(module);
}

void MIPSVisitor::print(const std::filesystem::path& output)
{
//	std::ofstream stream(output);
	module.print(std::cout);
//	module.print(stream);
//	stream.close();
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
		currentBlock->append(new li());
	}
	else {
		currentBlock->append(new move());
	}
}

void MIPSVisitor::visitAllocaInst(AllocaInst& I)
{
	currentFunction->addToStack(layout.getTypeAllocSize(I.getAllocatedType()));
}

void MIPSVisitor::visitStoreInst(StoreInst& I)
{
	if(auto i = dyn_cast<Constant>(I.getValueOperand())){
		currentBlock->append(new li());
	}

	currentBlock->append(new sw());
}
