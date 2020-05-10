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
	//TODO globals
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
	std::cout << I.getValueID() << std::endl;
}

void MIPSVisitor::visitAllocaInst(AllocaInst& I)
{
	InstVisitor::visitAllocaInst(I);
}

void MIPSVisitor::visitStoreInst(StoreInst& I)
{
	InstVisitor::visitStoreInst(I);
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

}

void MIPSVisitor::visitBranchInst(BranchInst& I)
{
	InstVisitor::visitBranchInst(I);
}

void MIPSVisitor::visitBinaryOperator(BinaryOperator& I)
{

}

void MIPSVisitor::visitInstruction(llvm::Instruction& I)
{
	std::string str;
	llvm::raw_string_ostream rso(str);
	I.print(rso);
	throw InternalError("Forgot to implement IR instruction '"+str+"' in MIPS");
}

