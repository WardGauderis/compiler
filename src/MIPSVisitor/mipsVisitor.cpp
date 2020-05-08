//
// Created by ward on 5/7/20.
//

#include "mipsVisitor.h"
#include <fstream>
#include <llvm/Support/raw_ostream.h>
#include "../errors.h"

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

void MIPSVisitor::visitModule(llvm::Module& M)
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

void MIPSVisitor::visitICmpInst(ICmpInst& I)
{
	InstVisitor::visitICmpInst(I);
}

void MIPSVisitor::visitFCmpInst(FCmpInst& I)
{
	InstVisitor::visitFCmpInst(I);
}

void MIPSVisitor::visitLoadInst(LoadInst& I)
{
	//TODO sign extend?
	currentBlock->append(new l(layout.getTypeAllocSize(I.getType())==1));
}

void MIPSVisitor::visitAllocaInst(AllocaInst& I)
{
	currentFunction->addToStack(layout.getTypeAllocSize(I.getAllocatedType()));
}

void MIPSVisitor::visitStoreInst(StoreInst& I)
{
	if (auto i = dyn_cast<Constant>(I.getValueOperand())) {
		currentBlock->append(new li());
	}
	currentBlock->append(new s(layout.getTypeAllocSize(I.getValueOperand()->getType())==1));
}

void MIPSVisitor::visitGetElementPtrInst(GetElementPtrInst& I)
{
	InstVisitor::visitGetElementPtrInst(I);
}

void MIPSVisitor::visitPHINode(PHINode& I)
{
	InstVisitor::visitPHINode(I);
}

void MIPSVisitor::visitTruncInst(TruncInst& I)
{
	InstVisitor::visitTruncInst(I);
}

void MIPSVisitor::visitZExtInst(ZExtInst& I)
{
	InstVisitor::visitZExtInst(I);
}

void MIPSVisitor::visitSExtInst(SExtInst& I)
{

}

void MIPSVisitor::visitFPToUIInst(FPToUIInst& I)
{
	InstVisitor::visitFPToUIInst(I);
}

void MIPSVisitor::visitFPToSIInst(FPToSIInst& I)
{
	InstVisitor::visitFPToSIInst(I);
}

void MIPSVisitor::visitUIToFPInst(UIToFPInst& I)
{
	InstVisitor::visitUIToFPInst(I);
}

void MIPSVisitor::visitSIToFPInst(SIToFPInst& I)
{
	InstVisitor::visitSIToFPInst(I);
}

void MIPSVisitor::visitPtrToIntInst(PtrToIntInst& I)
{
	InstVisitor::visitPtrToIntInst(I);
}

void MIPSVisitor::visitIntToPtrInst(IntToPtrInst& I)
{
	InstVisitor::visitIntToPtrInst(I);
}

void MIPSVisitor::visitBitCastInst(BitCastInst& I)
{
	InstVisitor::visitBitCastInst(I);
}

void MIPSVisitor::visitCallInst(CallInst& I)
{
	InstVisitor::visitCallInst(I);
}

void MIPSVisitor::visitReturnInst(ReturnInst& I)
{
	//TODO void
	//TODO ret/syscall
	if (auto i = dyn_cast<Constant>(I.getReturnValue())) {
		currentBlock->append(new li());
	}
	else {
		currentBlock->append(new move());
	}
}

void MIPSVisitor::visitBranchInst(BranchInst& I)
{
	InstVisitor::visitBranchInst(I);
}

void MIPSVisitor::visitBinaryOperator(BinaryOperator& I)
{
	bool immediate = isa<Constant>(I.getOperand(0)) || isa<Constant>(I.getOperand(1));
	switch (I.getOpcode()) {
	case llvm::Instruction::Add:
		currentBlock->append(new bin(immediate, "add"));
		break;
	default:
		InstVisitor::visitBinaryOperator(I);
	}
}

void MIPSVisitor::visitInstruction(llvm::Instruction& I)
{
	std::string str;
	llvm::raw_string_ostream rso(str);
	I.print(rso);
	throw InternalError("Forgot to implement IR instruction '"+str+"' in MIPS");
}

