//
// Created by ward on 3/28/20.
//

#ifndef COMPILER_IRVISITOR_H
#define COMPILER_IRVISITOR_H

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <filesystem>
#include <ast/statements.h>

#include "ast/expressions.h"
#include "ast/node.h"

class IRVisitor {
public:
	explicit IRVisitor(const std::filesystem::path& input);

	void convertAST(const std::unique_ptr<Ast::Node>& root);

	void LLVMOptimize();

	void print(const std::filesystem::path& output);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void visitLiteral(const Ast::Literal& literal);

	void visitComment(const Ast::Comment& comment);

	void visitVariable(const Ast::Variable& variable);

	void visitScope(const Ast::Scope& scope);

	void visitBinaryExpr(const Ast::BinaryExpr& binaryExpr);

	void visitPostfixExpr(const Ast::PostfixExpr& postfixExpr);

	void visitPrefixExpr(const Ast::PrefixExpr& prefixExpr);

	void visitCastExpr(const Ast::CastExpr& castExpr);

	void visitAssignment(const Ast::Assignment& assignment);

	void visitDeclaration(const Ast::Declaration& declaration);

	void visitPrintfStatement(const Ast::PrintfStatement& printfStatement);

	void visitIfStatement(const Ast::IfStatement& ifStatement);

	void visitLoopStatement(const Ast::LoopStatement& loopStatement);

	void visitControlStatement(const Ast::ControlStatement& controlStatement);

	void visitReturnStatement(const Ast::ReturnStatement& returnStatement);

	void visitFunctionDefinition(const Ast::FunctionDefinition& functionDefinition);

	void visitFunctionCall(const Ast::FunctionCall& functionCall);

private:
	llvm::LLVMContext context;
	llvm::Module module;
	llvm::IRBuilder<> builder;

	std::unordered_map<std::string, llvm::AllocaInst*> variables;

	llvm::Value* ret;

	llvm::Value* cast(llvm::Value* value, llvm::Type* to);

	llvm::Value* increaseOrDecrease(bool inc, llvm::Value* input);

	llvm::Type* convertToIR(const Type& type);
};

#endif //COMPILER_IRVISITOR_H
