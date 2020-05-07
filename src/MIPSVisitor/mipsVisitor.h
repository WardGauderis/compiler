//
// Created by ward on 5/7/20.
//

#ifndef COMPILER_MIPSVISITOR_H
#define COMPILER_MIPSVISITOR_H

#include <llvm/IR/InstVisitor.h>
#include <iostream>
#include <filesystem>

class MIPSVisitor : public llvm::InstVisitor<MIPSVisitor> {
public:
	void convertIR(const llvm::Module& module);

	void print(const std::filesystem::path& output);
};

#endif //COMPILER_MIPSVISITOR_H
