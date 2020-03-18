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
			throw InternalError("literal type is not supported in IR");
		}
	}

	//TODO alloca
	llvm::Value* Variable::codegen() const
	{
		return nullptr;
	}

	llvm::Value* BinaryExpr::codegen() const
	{
		auto l = lhs->codegen();
		auto r = rhs->codegen();
		bool floatOperation = type().isFloatingType();

		if (floatOperation)
		{
			if(!lhs->type().isFloatingType());
			if(!rhs->type().isFloatingType());
		}
		else
		{
//			if(!lhs->type().isFloatingType())
		}


		using namespace llvm;
		if (operation=="*") return builder.CreateBinOp(floatOperation ? Instruction::FMul : Instruction::Mul, l, r);
		if (operation=="/") return builder.CreateBinOp(floatOperation ? Instruction::FDiv : Instruction::SDiv, l, r);
		if (operation=="%") return builder.CreateBinOp(Instruction::SRem, l, r);
		if (operation=="+") return builder.CreateBinOp(floatOperation ? Instruction::FAdd : Instruction::Add, l, r);
		if (operation=="-") return builder.CreateBinOp(floatOperation ? Instruction::FSub : Instruction::Sub, l, r);
		if (operation=="<")
			return floatOperation ? builder.CreateFCmp(CmpInst::FCMP_OLT, l, r) :
			       builder.CreateICmp(CmpInst::ICMP_SLT, l, r);
		if (operation=="<=")
			return floatOperation ? builder.CreateFCmp(CmpInst::FCMP_OLE, l, r) :
			       builder.CreateICmp(CmpInst::ICMP_SLE, l, r);
		if (operation==">")
			return floatOperation ? builder.CreateFCmp(CmpInst::FCMP_OGT, l, r) :
			       builder.CreateICmp(CmpInst::ICMP_SGT, l, r);
		if (operation==">=")
			return floatOperation ? builder.CreateFCmp(CmpInst::FCMP_OGE, l, r) :
			       builder.CreateICmp(CmpInst::ICMP_SGE, l, r);
		if (operation=="==")
			return floatOperation ? builder.CreateFCmp(CmpInst::FCMP_OEQ, l, r) :
			       builder.CreateICmp(CmpInst::ICMP_EQ, l, r);
		if (operation=="!=")
			return floatOperation ? builder.CreateFCmp(CmpInst::FCMP_UNE, l, r) :
			       builder.CreateICmp(CmpInst::ICMP_NE, l, r);
		if (operation=="&&") return builder.CreateBinOp(floatOperation ? Instruction::And : Instruction::Mul, l, r);
		if (operation=="||") return builder.CreateBinOp(floatOperation ? Instruction::Or : Instruction::Mul, l, r);
		//TODO or and float pointer
		throw InternalError("Operand type is not supported in IR");
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