//
// Created by ward on 3/17/20.
//

#include "ast.h"
#include "errors.h"
#include "type.h"
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <filesystem>
#include <unordered_map>

using namespace llvm;

static LLVMContext context;
static Module module("temp", context);
static IRBuilder builder(context);
static std::unordered_map<std::string, AllocaInst*> variables;

llvm::Type* ::Type::convertToIR() const
{
	if (isBaseType())
	{
		switch (getBaseType())
		{
		case BaseType::Char:
			return llvm::Type::getInt8Ty(context);
		case BaseType::Int:
			return llvm::Type::getInt32Ty(context);
		case BaseType::Float:
			return llvm::Type::getFloatTy(context);
		default:
			throw InternalError("type is not supported in IR");
		}
	}
	else
	{
		return llvm::PointerType::getUnqual((*std::get<Type*>(type)).convertToIR());
	}
}

namespace Ast {

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

	llvm::Value* Comment::codegen() const
	{
		//TODO attach metadata
		MDNode* m = MDNode::get(context, MDString::get(context, comment));
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
			return ConstantInt::get(builder.getInt8Ty(), std::get<char>(literal));
		case 2:
			return ConstantInt::get(builder.getInt32Ty(), std::get<int>(literal));
		case 4:
			return ConstantFP::get(builder.getFloatTy(), std::get<float>(literal));
		default:
			throw InternalError("type is not supported in IR");
		}
	}

	llvm::Value* Variable::codegen() const
	{
		return builder.CreateLoad(variables[(*entry).first], (*entry).first);
	}

	llvm::Value* BinaryExpr::codegen() const
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

		if (operation.type==BinaryOperation::Mul)
			return builder.CreateBinOp(floatOperation ? Instruction::FMul : Instruction::Mul, l, r, "mul");
		if (operation.type==BinaryOperation::Div)
			return builder.CreateBinOp(floatOperation ? Instruction::FDiv : Instruction::SDiv, l, r, "div");
		if (operation.type==BinaryOperation::Mod) return builder.CreateBinOp(Instruction::SRem, l, r, "mod");
		if (operation.type==BinaryOperation::Add)
			return builder.CreateBinOp(floatOperation ? Instruction::FAdd : Instruction::Add, l, r, "add");
		if (operation.type==BinaryOperation::Sub)
			return builder.CreateBinOp(floatOperation ? Instruction::FSub : Instruction::Sub, l, r, "sub");
		if (operation.type==BinaryOperation::Lt)
			return floatOperation ? builder.CreateFCmp(CmpInst::FCMP_OLT, l, r, "lt") :
			       builder.CreateICmp(CmpInst::ICMP_SLT, l, r, "lt");
		if (operation.type==BinaryOperation::Le)
			return floatOperation ? builder.CreateFCmp(CmpInst::FCMP_OLE, l, r, "le") :
			       builder.CreateICmp(CmpInst::ICMP_SLE, l, r, "le");
		if (operation.type==BinaryOperation::Gt)
			return floatOperation ? builder.CreateFCmp(CmpInst::FCMP_OGT, l, r, "gt") :
			       builder.CreateICmp(CmpInst::ICMP_SGT, l, r, "gt");
		if (operation.type==BinaryOperation::Ge)
			return floatOperation ? builder.CreateFCmp(CmpInst::FCMP_OGE, l, r, "ge") :
			       builder.CreateICmp(CmpInst::ICMP_SGE, l, r, "ge");
		if (operation.type==BinaryOperation::Eq)
			return floatOperation ? builder.CreateFCmp(CmpInst::FCMP_OEQ, l, r, "eq") :
			       builder.CreateICmp(CmpInst::ICMP_EQ, l, r, "eq");
		if (operation.type==BinaryOperation::Neq)
			return floatOperation ? builder.CreateFCmp(CmpInst::FCMP_UNE, l, r, "neq") :
			       builder.CreateICmp(CmpInst::ICMP_NE, l, r, "nq");
		if (operation.type==BinaryOperation::And)//TODO logical
			return builder.CreateBinOp(floatOperation ? Instruction::And : Instruction::Mul, l, r, "and");
		if (operation.type==BinaryOperation::Or)
			return builder.CreateBinOp(floatOperation ? Instruction::Or : Instruction::Mul, l, r, "or");
		//TODO or and, float pointer
		throw InternalError("type is not supported in IR");
	}

	llvm::Value* increaseOrDecrease(bool inc, const ::Type& type, llvm::Value* input, const std::string& name)
	{
		llvm::Value* temp;
		std::string opName = inc ? "inc" : "dec";
		if (type.isFloatingType())
			temp = builder.CreateBinOp(inc ? Instruction::FAdd : Instruction::FSub, input,
					ConstantInt::get(builder.getFloatTy(), 1), opName);
		else if (type.isIntegerType())
			temp = builder.CreateBinOp(inc ? Instruction::Add : Instruction::Sub, input,
					builder.getInt32(1), opName);
		else if (type.isCharacterType())
			temp = builder.CreateBinOp(inc ? Instruction::Add : Instruction::Sub, input,
					builder.getInt8(1), opName);
		else if (type.isPointerType())
			temp = builder.CreateInBoundsGEP(input->getType(), input, builder.getInt32(inc*2-1), opName);
		else throw InternalError("type is not supported in IR");
		return builder.CreateStore(temp, variables[name]);
	}

	llvm::Value* PostfixExpr::codegen() const
	{
		auto result = operand->codegen();
		bool inc = operation.type==PostfixOperation::Incr;
		increaseOrDecrease(inc, type(), result, name());
		return result;
	}

	llvm::Value* PrefixExpr::codegen() const
	{
		auto result = operand->codegen();
		if (operation.type==PrefixOperation::Incr) return increaseOrDecrease(true, type(), result, name());
		else if (operation.type==PrefixOperation::Decr) return increaseOrDecrease(false, type(), result, name());
		else if (operation.type==PrefixOperation::Plus) return result;
		else if (operation.type==PrefixOperation::Neg)
		{
			auto type = result->getType();
			if (type->isFloatTy())
				return builder.CreateFSub(llvm::ConstantFP::get(type, 0), result, "neg");
			else if (type->isIntegerTy())
				return builder.CreateSub(llvm::ConstantInt::get(type, 0), result, "neg");
			else throw InternalError("type is not supported in IR");
		}
		else if (operation.type==PrefixOperation::Not)
		{
			return builder.CreateNot(result, "not"); //TODO float?
		}
		else throw InternalError("type is not supported in IR");
	}

	llvm::Value* cast(llvm::Value* value, llvm::Type* to)
	{
		auto from = value->getType();
		if (from==to) return value;
		if (from->isIntegerTy() && to->isIntegerTy()) return builder.CreateSExtOrTrunc(value, to);
		if (from->isIntegerTy() && to->isFloatTy()) return builder.CreateSIToFP(value, to);
		if (from->isFloatTy() && to->isIntegerTy()) return builder.CreateFPToSI(value, to);
		if (from->isPointerTy() && to->isPointerTy()) return builder.CreatePointerCast(value, to);
		if (from->isPointerTy() && to->isIntegerTy()) return builder.CreatePtrToInt(value, to);
		if (from->isPointerTy() && to->isIntegerTy()) return builder.CreatePtrToInt(value, to);
	}

	llvm::Value* CastExpr::codegen() const
	{
		auto toCast = operand->codegen();
		auto from = toCast->getType();
		auto to = type().convertToIR();
		return nullptr;
	}

	llvm::Value* Assignment::codegen() const
	{
		auto result = expr->codegen();
		builder.CreateStore(result, variables[variable->name()]);
		return result;
	}

	llvm::Value* Declaration::codegen() const
	{
		auto result = builder.CreateAlloca(variable->type().convertToIR(), nullptr, variable->name());
		variables[variable->name()] = result;
		if (expr) builder.CreateStore(expr->codegen(), result);
		return result;
	}

	llvm::Value* PrintfStatement::codegen() const
	{
		auto result = expr->codegen();
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
				{builder.CreateGlobalStringPtr(format, name), result});//TODO multilple calls
	}
}
