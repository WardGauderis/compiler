//
// Created by ward on 3/17/20.
//

#include "ast.h"
#include "errors.h"
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Verifier.h>
#include <filesystem>

namespace Ast {

	static llvm::LLVMContext context;
	static llvm::IRBuilder builder(context);
	static std::map<std::string, llvm::AllocaInst*> variables;

	void ast2ir(const std::unique_ptr<Ast::Node>& root, std::filesystem::path& path)
	{
		llvm::Module module(path.string(), context);
		llvm::FunctionCallee callee = module.getOrInsertFunction("main", llvm::Type::getInt32Ty(context));
		auto function = llvm::cast<llvm::Function>(callee.getCallee());
		function->setCallingConv(llvm::CallingConv::C);
		auto block = llvm::BasicBlock::Create(context, "entry", function);
		builder.SetInsertPoint(block);

		builder.CreateRet(llvm::ConstantInt::get(builder.getInt32Ty(), 0));
		llvm::verifyFunction(*function, &llvm::errs());
		llvm::verifyModule(module, &llvm::errs());
		module.print(llvm::outs(), nullptr, false, true);
	}

	llvm::Value* Comment::codegen() const
	{
		return nullptr;
	}

	llvm::Value* Block::codegen() const
	{
		for (const auto& node: nodes)
		{
			node->codegen();
		}
		return nullptr;
	}

	llvm::Value* Literal::codegen() const
	{
		switch (literal.index())
		{
		case 0:
			return llvm::ConstantInt::get(builder.getInt8Ty(), std::get<char>(literal));
		case 2:
			return llvm::ConstantInt::get(builder.getInt32Ty(), std::get<int>(literal));
		case 4:
			return llvm::ConstantFP::get(builder.getFloatTy(), std::get<float>(literal));
		default:
			throw InternalError("literal type is not supported yet in IR");
		}
	}

	//TODO alloca
	llvm::Value* Variable::codegen() const
	{
		return nullptr;
	}

	llvm::Value* BinaryExpr::codegen() const
	{
//		auto l = lhs->codegen();
//		auto r = rhs->codegen();
//		if (operation == "*") return builder.CreateBinOp()
//		if (operation == "/")
//		if (operation == "%")
//		if (operation == "+")
//		if (operation == "<")
//		if (operation == "<=")
//		if (operation == ">")
//		if (operation == ">=")
//		if (operation == "==")
//		if (operation == "!=")
//		if (operation == "&&")
//		if (operation == "||")
	}

	llvm::Value* PostfixExpr::codegen() const
	{
		return nullptr;
	}

	llvm::Value* PrefixExpr::codegen() const
	{
		return nullptr;
	}

	llvm::Value* UnaryExpr::codegen() const
	{
		return nullptr;
	}

	llvm::Value* CastExpr::codegen() const
	{
		return nullptr;
	}

	llvm::Value* Assignment::codegen() const
	{
		return nullptr;
	}

	llvm::Value* Declaration::codegen() const
	{
		return nullptr;
	}

	llvm::Value* PrintfStatement::codegen() const
	{
		return nullptr;
	}
}