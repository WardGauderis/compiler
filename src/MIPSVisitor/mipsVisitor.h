//
// Created by ward on 5/7/20.
//

#ifndef COMPILER_MIPSVISITOR_H
#define COMPILER_MIPSVISITOR_H

#include <llvm/IR/InstVisitor.h>
#include <filesystem>
#include "mips.h"

class MIPSVisitor : public llvm::InstVisitor<MIPSVisitor> {
public:
	MIPSVisitor();

	void convertIR(llvm::Module& module);

	void print(const std::filesystem::path& output);

	[[maybe_unused]] void visitModule(llvm::Module& M);

	[[maybe_unused]] void visitFunction(llvm::Function& F);

	[[maybe_unused]] void visitBasicBlock(llvm::BasicBlock& BB);

	[[maybe_unused]] void visitCmpInst(llvm::CmpInst& I);

	[[maybe_unused]] void visitLoadInst(llvm::LoadInst& I);

	[[maybe_unused]] void visitAllocaInst(llvm::AllocaInst& I);

	[[maybe_unused]] void visitStoreInst(llvm::StoreInst& I);

	[[maybe_unused]] void visitGetElementPtrInst(llvm::GetElementPtrInst& I);

	[[maybe_unused]] void visitPHINode(llvm::PHINode& I);

	[[maybe_unused]] void visitTruncInst(llvm::TruncInst& I);

	[[maybe_unused]] void visitZExtInst(llvm::ZExtInst& I);

	[[maybe_unused]] void visitSExtInst(llvm::SExtInst& I);

	[[maybe_unused]] void visitFPToUIInst(llvm::FPToUIInst& I);

	[[maybe_unused]] void visitFPToSIInst(llvm::FPToSIInst& I);

	[[maybe_unused]] void visitUIToFPInst(llvm::UIToFPInst& I);

	[[maybe_unused]] void visitSIToFPInst(llvm::SIToFPInst& I);

	[[maybe_unused]] void visitPtrToIntInst(llvm::PtrToIntInst& I);

	[[maybe_unused]] void visitIntToPtrInst(llvm::IntToPtrInst& I);

	[[maybe_unused]] void visitBitCastInst(llvm::BitCastInst& I);

	[[maybe_unused]] void visitCallInst(llvm::CallInst& I);

	[[maybe_unused]] void visitReturnInst(llvm::ReturnInst& I);

	[[maybe_unused]] void visitBranchInst(llvm::BranchInst& I);

	[[maybe_unused]] void visitBinaryOperator(llvm::BinaryOperator& I);

	[[maybe_unused]] void visitInstruction(llvm::Instruction& I);

private:
	mips::Module module;
	mips::Function* currentFunction;
	mips::Block* currentBlock;
};

#endif //COMPILER_MIPSVISITOR_H
