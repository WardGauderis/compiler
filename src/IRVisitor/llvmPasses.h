//
// Created by ward on 5/17/20.
//

#ifndef COMPILER_LLVMPASSES_H
#define COMPILER_LLVMPASSES_H

#include <llvm/IR/PassManager.h>

using namespace llvm;

class RemoveUnusedCodeInBlockPass : public llvm::FunctionPass {
public:
	static char ID;

	RemoveUnusedCodeInBlockPass()
			:FunctionPass(ID) { }

	bool runOnFunction(llvm::Function& function) final
	{
		for (auto& block: function) {
			bool done = false;
			for (auto instruction = block.begin(); instruction!=block.end();) {
				if (done) instruction = instruction->eraseFromParent();
				else {
					done |= instruction->isTerminator();
					++instruction;
				}
			}
		}
		return false;
	}
};

/// based on LLVM lib/Transforms/Utils/DemoteRegToStack.cpp and lib/Transforms/Scalar/Reg2mem.cpp

AllocaInst* DemotePHIToStack(PHINode* P, Instruction* AllocaPoint)
{
	if (P->use_empty()) {
		P->eraseFromParent();
		return nullptr;
	}
	const DataLayout& DL = P->getModule()->getDataLayout();
	AllocaInst* Slot;
	if (AllocaPoint) {
		Slot = new AllocaInst(P->getType(), DL.getAllocaAddrSpace(), nullptr,
				P->getName()+".reg2mem", AllocaPoint);
	}
	else {
		Function* F = P->getParent()->getParent();
		Slot = new AllocaInst(P->getType(), DL.getAllocaAddrSpace(), nullptr,
				P->getName()+".reg2mem",
				&F->getEntryBlock().front());
	}
	for (unsigned i = 0, e = P->getNumIncomingValues(); i<e; ++i) {
		if (InvokeInst* II = dyn_cast<InvokeInst>(P->getIncomingValue(i))) {
			assert(II->getParent()!=P->getIncomingBlock(i) &&
					"Invoke edge not supported yet");
			(void) II;
		}
		new StoreInst(P->getIncomingValue(i), Slot,
				P->getIncomingBlock(i)->getTerminator());
	}
	BasicBlock::iterator InsertPt = P->getIterator();
	for (; isa<PHINode>(InsertPt) || InsertPt->isEHPad(); ++InsertPt);
	Value* V =
			new LoadInst(P->getType(), Slot, P->getName()+".reload", &*InsertPt);
	P->replaceAllUsesWith(V);
	P->eraseFromParent();
	return Slot;
}

class RemovePhiInstructionPass : public llvm::FunctionPass {
public:
	static char ID;

	RemovePhiInstructionPass()
			:FunctionPass(ID) { }

	bool runOnFunction(llvm::Function& F) final
	{
		if (F.isDeclaration() || skipFunction(F))
			return false;
		BasicBlock* BBEntry = &F.getEntryBlock();
		BasicBlock::iterator I = BBEntry->begin();
		while (isa<AllocaInst>(I)) ++I;
		CastInst* AllocaInsertionPoint = new BitCastInst(
				Constant::getNullValue(llvm::Type::getInt32Ty(F.getContext())),
				llvm::Type::getInt32Ty(F.getContext()), "reg2mem alloca point", &*I);

		std::list<Instruction*> WorkList;
		for (BasicBlock& ibb : F)
			for (BasicBlock::iterator iib = ibb.begin(), iie = ibb.end(); iib!=iie;
			     ++iib)
				if (isa<PHINode>(iib))
					WorkList.push_front(&*iib);
		for (Instruction* ilb : WorkList)
			DemotePHIToStack(cast<PHINode>(ilb), AllocaInsertionPoint);
		return true;
	}
};

#endif //COMPILER_LLVMPASSES_H
