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
	MIPSVisitor(const llvm::Module& module);

	void convertIR(llvm::Module& module);

	void print(const std::filesystem::path& output);

	[[maybe_unused]] void visitModule(llvm::Module& M);

	[[maybe_unused]] void visitFunction(llvm::Function& F);

	[[maybe_unused]] void visitBasicBlock(llvm::BasicBlock& BB);

	[[maybe_unused]] void visitReturnInst(llvm::ReturnInst& I);

	[[maybe_unused]] void visitAllocaInst(llvm::AllocaInst& I);

	[[maybe_unused]] void visitStoreInst(llvm::StoreInst& I);

private:
	mips::Module module;
	mips::Function* currentFunction;
	mips::Block* currentBlock;

	const llvm::DataLayout layout;
};

#endif //COMPILER_MIPSVISITOR_H
