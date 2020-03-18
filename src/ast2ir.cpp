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

using namespace llvm;

namespace Ast {

	static LLVMContext context;
	static Module module("temp", context);
	static IRBuilder builder(context);

	void ast2ir(const std::unique_ptr<Ast::Node>& root, std::filesystem::path& path)
	{
		module.getOrInsertFunction("printf",
				llvm::FunctionType::get(llvm::Type::getInt32PtrTy(context), builder.getInt8PtrTy(), true));

		FunctionCallee tmp = module.getOrInsertFunction("main", builder.getInt32Ty());
		auto main = cast<Function>(tmp.getCallee());

		auto block = BasicBlock::Create(context, "entry", main);
		builder.SetInsertPoint(block);

		root->codegen();

		builder.CreateRet(ConstantInt::get(builder.getInt32Ty(), 0));

		verifyFunction(*main, &errs());
		verifyModule(module, &errs());
		module.print(outs(), nullptr, false, true);
	}

	Value* Comment::codegen() const
	{
		return nullptr;
	}

	Value* Block::codegen() const
	{
		for (const auto& node: nodes)
		{
			node->codegen();
		}
		return nullptr;
	}

	Value* Literal::codegen() const
	{
		switch (literal.index())
		{
		case 0:
			return ConstantInt::get(builder.getInt8Ty(), std::get<char>(literal));
		case 2:
			return ConstantInt::get(builder.getInt32Ty(), std::get<int>(literal));
		case 4:
			return ConstantFP::get(builder.getFloatTy(), std::get<float>(literal));
		default:
			throw InternalError("type is not supported in IR");
		}
	}

	//TODO alloca
	Value* Variable::codegen() const
	{
		return nullptr;
	}

	Value* BinaryExpr::codegen() const
	{
		auto l = lhs->codegen();
		auto r = rhs->codegen();
		bool floatOperation = type().isFloatingType();

		if (floatOperation)
		{
			if (!lhs->type().isFloatingType()) l = builder.CreateSIToFP(l, builder.getFloatTy());
			if (!rhs->type().isFloatingType()) r = builder.CreateSIToFP(r, builder.getFloatTy());
		}
		else
		{
			if (lhs->type().isCharacterType()) l = builder.CreateSExt(l, builder.getInt32Ty());
			if (rhs->type().isCharacterType()) r = builder.CreateSExt(l, builder.getInt32Ty());
		}

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
		throw InternalError("type is not supported in IR");
	}

	Value* PostfixExpr::codegen() const
	{
		return nullptr;
	}

	Value* PrefixExpr::codegen() const
	{
		return nullptr;
	}

	Value* CastExpr::codegen() const
	{
		return nullptr;
	}

	Value* Assignment::codegen() const
	{
		return nullptr;
	}

	Value* Declaration::codegen() const
	{
		return nullptr;
	}

	Value* PrintfStatement::codegen() const
	{
		Value* result = expr->codegen();
		std::string format;
		std::string name;
		if (expr->type().isPointerType())
		{
			format = "%p\n";
			name = "ptrFormat";
		}
		else if (expr->type().isFloatingType())
		{
			format = "%f\n";
			name = "floatFormat";
		}
		else if (expr->type().isCharacterType())
		{
			format = "%c\n";
			name = "charFormat";
		}
		else if (expr->type().isIntegerType())
		{
			format = "%d\n";
			name = "intFormat";
		}
		else throw InternalError("ype is not supported in IR");
		return builder.CreateCall(module.getFunction("printf"),
				{builder.CreateGlobalStringPtr(format, name), result});
	}
}