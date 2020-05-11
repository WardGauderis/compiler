//
// Created by ward on 5/7/20.
//

#include "mipsVisitor.h"
#include <fstream>
#include <llvm/Support/raw_ostream.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Support/SourceMgr.h>
#include "../errors.h"

using namespace llvm;
using namespace mips;

MIPSVisitor::MIPSVisitor() { }

void MIPSVisitor::convertIR(llvm::Module& module)
{
	SMDiagnostic Err;
	llvm::LLVMContext context;
	auto m = parseIRFile("bubble.ll", Err, context);
	visit(*m);
//	visit(module);
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
	//TODO signature
	currentFunction = new mips::Function();
	module.append(currentFunction);
}

void MIPSVisitor::visitBasicBlock(BasicBlock& BB)
{
	//TODO label
	currentBlock = new mips::Block();
	currentFunction->append(currentBlock);
}

void MIPSVisitor::visitICmpInst(ICmpInst& I)
{
	//TODO types
	InstVisitor::visitICmpInst(I);
	switch (I.getPredicate()) {
	case CmpInst::FCMP_FALSE:
		break;
	case CmpInst::FCMP_OEQ:
		break;
	case CmpInst::FCMP_OGT:
		break;
	case CmpInst::FCMP_OGE:
		break;
	case CmpInst::FCMP_OLT:
		break;
	case CmpInst::FCMP_OLE:
		break;
	case CmpInst::FCMP_ONE:
		break;
	case CmpInst::FCMP_ORD:
		break;
	case CmpInst::FCMP_UNO:
		break;
	case CmpInst::FCMP_UEQ:
		break;
	case CmpInst::FCMP_UGT:
		break;
	case CmpInst::FCMP_UGE:
		break;
	case CmpInst::FCMP_ULT:
		break;
	case CmpInst::FCMP_ULE:
		break;
	case CmpInst::FCMP_UNE:
		break;
	case CmpInst::FCMP_TRUE:
		break;
	case CmpInst::BAD_FCMP_PREDICATE:
		break;
	case CmpInst::ICMP_EQ:
		break;
	case CmpInst::ICMP_NE:
		break;
	case CmpInst::ICMP_UGT:
		break;
	case CmpInst::ICMP_UGE:
		break;
	case CmpInst::ICMP_ULT:
		break;
	case CmpInst::ICMP_ULE:
		break;
	case CmpInst::ICMP_SGT:
		break;
	case CmpInst::ICMP_SGE:
		break;
	case CmpInst::ICMP_SLT:
		break;
	case CmpInst::ICMP_SLE:
		break;
	case CmpInst::BAD_ICMP_PREDICATE:
		break;
	}
}

void MIPSVisitor::visitFCmpInst(FCmpInst& I)
{
	//TODO types
	InstVisitor::visitFCmpInst(I);
	switch (I.getPredicate()) {
	case CmpInst::FCMP_FALSE:
		break;
	case CmpInst::FCMP_OEQ:
		break;
	case CmpInst::FCMP_OGT:
		break;
	case CmpInst::FCMP_OGE:
		break;
	case CmpInst::FCMP_OLT:
		break;
	case CmpInst::FCMP_OLE:
		break;
	case CmpInst::FCMP_ONE:
		break;
	case CmpInst::FCMP_ORD:
		break;
	case CmpInst::FCMP_UNO:
		break;
	case CmpInst::FCMP_UEQ:
		break;
	case CmpInst::FCMP_UGT:
		break;
	case CmpInst::FCMP_UGE:
		break;
	case CmpInst::FCMP_ULT:
		break;
	case CmpInst::FCMP_ULE:
		break;
	case CmpInst::FCMP_UNE:
		break;
	case CmpInst::FCMP_TRUE:
		break;
	case CmpInst::BAD_FCMP_PREDICATE:
		break;
	case CmpInst::ICMP_EQ:
		break;
	case CmpInst::ICMP_NE:
		break;
	case CmpInst::ICMP_UGT:
		break;
	case CmpInst::ICMP_UGE:
		break;
	case CmpInst::ICMP_ULT:
		break;
	case CmpInst::ICMP_ULE:
		break;
	case CmpInst::ICMP_SGT:
		break;
	case CmpInst::ICMP_SGE:
		break;
	case CmpInst::ICMP_SLT:
		break;
	case CmpInst::ICMP_SLE:
		break;
	case CmpInst::BAD_ICMP_PREDICATE:
		break;
	}
}

void MIPSVisitor::visitLoadInst(LoadInst& I)
{
	//TODO label, register, stack
	InstVisitor::visitLoadInst(I);
}

void MIPSVisitor::visitAllocaInst(AllocaInst& I)
{
	//TODO stack function
	InstVisitor::visitAllocaInst(I);
}

void MIPSVisitor::visitStoreInst(StoreInst& I)
{
	//TODO label, regitster, stack
	InstVisitor::visitStoreInst(I);
}

void MIPSVisitor::visitGetElementPtrInst(GetElementPtrInst& I)
{
	//TODO immediate, addu
	InstVisitor::visitGetElementPtrInst(I);
}

void MIPSVisitor::visitPHINode(PHINode& I)
{
	//TODO to if statement
	InstVisitor::visitPHINode(I);
}

void MIPSVisitor::visitTruncInst(TruncInst& I)
{

}

void MIPSVisitor::visitZExtInst(ZExtInst& I)
{

}

void MIPSVisitor::visitSExtInst(SExtInst& I)
{

}

void MIPSVisitor::visitFPToUIInst(FPToUIInst& I)
{
	//TODO cvt.w.s
	InstVisitor::visitFPToUIInst(I);
}

void MIPSVisitor::visitFPToSIInst(FPToSIInst& I)
{
	//TODO cvt.w.s
	InstVisitor::visitFPToSIInst(I);
}

void MIPSVisitor::visitUIToFPInst(UIToFPInst& I)
{
	//TODO cvt.s.w
	InstVisitor::visitUIToFPInst(I);
}

void MIPSVisitor::visitSIToFPInst(SIToFPInst& I)
{
	//TODO cvt.w.s
	InstVisitor::visitSIToFPInst(I);
}

void MIPSVisitor::visitPtrToIntInst(PtrToIntInst& I)
{

}

void MIPSVisitor::visitIntToPtrInst(IntToPtrInst& I)
{

}

void MIPSVisitor::visitBitCastInst(BitCastInst& I)
{

}

void MIPSVisitor::visitCallInst(CallInst& I)
{
	//TODO arg
	InstVisitor::visitCallInst(I);
}

void MIPSVisitor::visitReturnInst(ReturnInst& I)
{
	//TODO move
	InstVisitor::visitReturnInst(I);
}

void MIPSVisitor::visitBranchInst(BranchInst& I)
{
	//TODO branch
	InstVisitor::visitBranchInst(I);
}

void MIPSVisitor::visitBinaryOperator(BinaryOperator& I)
{
	//TODO enum
	InstVisitor::visitBinaryOperator(I);
	switch (I.getOpcode()) {
	case llvm::Instruction::FAdd:
		break;
	case llvm::Instruction::Sub:
		break;
	case llvm::Instruction::FSub:
		break;
	case llvm::Instruction::Mul:
		break;
	case llvm::Instruction::FMul:
		break;
	case llvm::Instruction::UDiv:
		break;
	case llvm::Instruction::SDiv:
		break;
	case llvm::Instruction::FDiv:
		break;
	case llvm::Instruction::URem:
		break;
	case llvm::Instruction::SRem:
		break;
	case llvm::Instruction::FRem:
		break;
	case llvm::Instruction::Shl:
		break;
	case llvm::Instruction::LShr:
		break;
	case llvm::Instruction::AShr:
		break;
	case llvm::Instruction::And:
		break;
	case llvm::Instruction::Or:
		break;
	case llvm::Instruction::Xor:
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
	std::cout << InternalError("Forgot to implement IR instruction '"+str+"' in MIPS");
}

