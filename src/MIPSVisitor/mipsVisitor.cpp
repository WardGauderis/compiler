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
	currentFunction = new mips::Function(&F);
	module.append(currentFunction);
}

void MIPSVisitor::visitBasicBlock(BasicBlock& BB)
{
	//TODO label
	currentBlock = new mips::Block(&BB);
	currentFunction->append(currentBlock);
}

void MIPSVisitor::visitCmpInst(CmpInst& I)
{
	const auto& a = &I;
	const auto& b = I.getOperand(0);
	const auto& c = I.getOperand(1);
	mips::Instruction* instruction;
	switch (I.getPredicate()) {
	case CmpInst::FCMP_OEQ:
	case CmpInst::FCMP_UEQ:
		instruction = new mips::Arithmetic("c.eq.s", a, b, c);
		break;
	case CmpInst::FCMP_OGT:
	case CmpInst::FCMP_UGT:
		instruction = new mips::Arithmetic("c.lt.s", a, c, b);
		break;
	case CmpInst::FCMP_OGE:
	case CmpInst::FCMP_UGE:
		instruction = new mips::Arithmetic("c.le.s", a, c, b);
		break;
	case CmpInst::FCMP_OLT:
	case CmpInst::FCMP_ULT:
		instruction = new mips::Arithmetic("c.lt.s", a, b, c);
		break;
	case CmpInst::FCMP_OLE:
	case CmpInst::FCMP_ULE:
		instruction = new mips::Arithmetic("c.le.s", a, b, c);
		break;
	case CmpInst::FCMP_ONE:
	case CmpInst::FCMP_UNE:
//		instruction = new mips::Not(); //TODO not
		instruction = new mips::Arithmetic("c.eq.s", a, b, c);
		break;
	case CmpInst::ICMP_EQ:
		instruction = new mips::Arithmetic("seq", a, b, c);
		break;
	case CmpInst::ICMP_NE:
		instruction = new mips::Arithmetic("sne", a, b, c);
		break;
	case CmpInst::ICMP_UGT:
		instruction = new mips::Arithmetic("sgtu", a, b, c);
		break;
	case CmpInst::ICMP_UGE:
		instruction = new mips::Arithmetic("sgeu", a, b, c);
		break;
	case CmpInst::ICMP_ULT:
		instruction = new mips::Arithmetic("slte", a, b, c);
		break;
	case CmpInst::ICMP_ULE:
		instruction = new mips::Arithmetic("slue", a, b, c);
		break;
	case CmpInst::ICMP_SGT:
		instruction = new mips::Arithmetic("sgt", a, b, c);
		break;
	case CmpInst::ICMP_SGE:
		instruction = new mips::Arithmetic("sge", a, b, c);
		break;
	case CmpInst::ICMP_SLT:
		instruction = new mips::Arithmetic("slt", a, b, c);
		break;
	case CmpInst::ICMP_SLE:
		instruction = new mips::Arithmetic("sle", a, b, c);
		break;
	default:
		instruction = nullptr;
		InstVisitor::visitCmpInst(I);
		break;
	}
	currentBlock->append(instruction);
}

void MIPSVisitor::visitLoadInst(LoadInst& I)
{
	//TODO label, register, stack
	currentBlock->append(new mips::Load(&I, I.getPointerOperand()));
}

void MIPSVisitor::visitAllocaInst(AllocaInst& I)
{
	//TODO stack function
	InstVisitor::visitAllocaInst(I);
}

void MIPSVisitor::visitStoreInst(StoreInst& I)
{
	//TODO label, regitster, stack
	currentBlock->append(new mips::Store(&I, I.getPointerOperand()));
	InstVisitor::visitStoreInst(I);
}

void MIPSVisitor::visitGetElementPtrInst(GetElementPtrInst& I)
{
	//TODO immediate, addu
	InstVisitor::visitGetElementPtrInst(I);
}

void MIPSVisitor::visitPHINode(PHINode& I)
{
	for (const auto& block: I.blocks()) {
		const auto& value = I.getIncomingValueForBlock(block);
		const auto& mipsBlock = currentFunction->getBlockByBasicBlock(block);
		mips::Instruction* instruction;
		const auto& constant = dyn_cast<ConstantFP>(value);
		if (const auto& constant = dyn_cast<ConstantInt>(value)) {
			instruction = new mips::Load(&I, constant);
		}
		else {
			instruction = new mips::Move(&I, value);
		}
		mipsBlock->appendBeforeLast(instruction);
	}
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
//	currentBlock->append(new mips::)
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
	const auto& a = &I;
	const auto& b = I.getOperand(0);
	const auto& c = I.getOperand(1);
	mips::Instruction* instruction;

	switch (I.getOpcode()) {
	case llvm::Instruction::FAdd:
		instruction = new mips::Arithmetic("add", a, b, c);
		break;
	case llvm::Instruction::Sub:
		instruction = new mips::Arithmetic("add", a, b, c);
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

	currentBlock->append(instruction);
}

void MIPSVisitor::visitInstruction(llvm::Instruction& I)
{
	std::string str;
	llvm::raw_string_ostream rso(str);
	I.print(rso);
	std::cout << InternalError("Forgot to implement IR instruction '"+str+"' in MIPS");
}

