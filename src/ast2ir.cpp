//
// Created by ward on 3/17/20.
//

#include "ast/node.h"
#include "ast/expressions.h"
#include "ast/statements.h"


#include "errors.h"
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/PassManager.h>
#include <llvm/Passes/PassBuilder.h>

using namespace llvm;

static LLVMContext context;
static std::unique_ptr<Module> module;
static std::unique_ptr<IRBuilder<>> builder;
static std::unique_ptr<std::unordered_map<std::string, AllocaInst*>> variables;

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

	void ast2ir(const std::unique_ptr<Ast::Node>& root, const std::filesystem::path& input,
			const std::filesystem::path& output, bool optimised)
	{
		module = std::make_unique<Module>(input.string(), context);
		builder = std::make_unique<IRBuilder<>>(context);
		variables = std::make_unique<std::unordered_map<std::string, AllocaInst*>>();

		module->getOrInsertFunction("printf",
				llvm::FunctionType::get(llvm::Type::getInt32PtrTy(context), builder->getInt8PtrTy(), true));

		FunctionCallee tmp = module->getOrInsertFunction("main", builder->getInt32Ty());
		auto main = cast<Function>(tmp.getCallee());

		auto block = BasicBlock::Create(context, "entry", main);
		builder->SetInsertPoint(block);

		root->codegen();

		builder->CreateRet(ConstantInt::get(builder->getInt32Ty(), 0));

		verifyFunction(*main, &errs());
		verifyModule(*module, &errs());

		if (optimised)
		{
			llvm::PassBuilder passBuilder;
			llvm::LoopAnalysisManager loopAnalysisManager(true);
			llvm::FunctionAnalysisManager functionAnalysisManager(true);
			llvm::CGSCCAnalysisManager cGSCCAnalysisManager(true);
			llvm::ModuleAnalysisManager moduleAnalysisManager(true);
			passBuilder.registerModuleAnalyses(moduleAnalysisManager);
			passBuilder.registerCGSCCAnalyses(cGSCCAnalysisManager);
			passBuilder.registerFunctionAnalyses(functionAnalysisManager);
			passBuilder.registerLoopAnalyses(loopAnalysisManager);
			passBuilder.crossRegisterProxies(loopAnalysisManager, functionAnalysisManager, cGSCCAnalysisManager,
					moduleAnalysisManager);
			llvm::ModulePassManager modulePassManager = passBuilder.buildPerModuleDefaultPipeline(
					llvm::PassBuilder::OptimizationLevel::O3);
			modulePassManager.run(*module, moduleAnalysisManager);
		}

		std::error_code ec;
		raw_fd_ostream out(output.string(), ec);
		module->print(out, nullptr, false, true);
	}

	llvm::Value* cast(llvm::Value* value, llvm::Type* to)
	{
		auto from = value->getType();
		if (from==to) return value;
		if (from->isIntegerTy())
		{
			if (to->isIntegerTy()) return builder->CreateSExtOrTrunc(value, to);
			else if (to->isFloatTy()) return builder->CreateSIToFP(value, to);
			else if (to->isPointerTy()) return builder->CreateIntToPtr(value, to);
			else throw InternalError("type is not supported in IR");
		}
		else if (from->isFloatTy())
		{
			if (to->isIntegerTy()) return builder->CreateFPToSI(value, to);
			else throw InternalError("type is not supported in IR");
		}
		else if (from->isPointerTy())
		{
			if (to->isIntegerTy()) return builder->CreatePtrToInt(value, to);
			if (to->isPointerTy()) return builder->CreatePointerCast(value, to);
			else throw InternalError("type is not supported in IR");
		}
		throw InternalError("cast is not supported in IR");
	}

	llvm::Value* Comment::codegen() const
	{
		MDNode* m = MDNode::get(context, MDString::get(context, comment));
		return nullptr;
	}

	llvm::Value* Scope::codegen() const
	{
		for (const auto& statement: statements)
		{
            statement->codegen();
		}
		return nullptr;
	}

	llvm::Value* Literal::codegen() const
	{
		switch (literal.index())
		{
		case 0:
			return ConstantInt::get(builder->getInt8Ty(), std::get<char>(literal));
		case 2:
			return ConstantInt::get(builder->getInt32Ty(), std::get<int>(literal));
		case 4:
			return ConstantFP::get(builder->getFloatTy(), std::get<float>(literal));
		default:
			throw InternalError("type is not supported in IR");
		}
	}

	llvm::Value* Variable::codegen() const
	{
		return builder->CreateLoad((*variables)[identifier]);
	}

	llvm::Value* BinaryExpr::codegen() const
	{
		auto l = lhs->codegen();
		auto r = rhs->codegen();
		auto resultType = type().convertToIR();

		if (operation.type==BinaryOperation::Add && l->getType()->isPointerTy())
			return builder->CreateInBoundsGEP(l, r);
		else if (operation.type==BinaryOperation::Add && r->getType()->isPointerTy())
			return builder->CreateInBoundsGEP(r, l);
		else if (operation.type==BinaryOperation::Sub && l->getType()->isPointerTy())
		{
			auto temp = builder->CreateSub(ConstantInt::get(r->getType(), 0), r);
			return builder->CreateInBoundsGEP(l, temp);
		}

		bool floatOperation = false;
		if (l->getType()->isFloatTy() || r->getType()->isFloatTy())
		{
			l = cast(l, builder->getFloatTy());
			r = cast(r, builder->getFloatTy());
			floatOperation = true;
		}
		else if (l->getType()->isIntegerTy() || r->getType()->isIntegerTy())
		{
			l = cast(l, builder->getInt32Ty());
			r = cast(r, builder->getInt32Ty());
		}
		else if (l->getType()->isPointerTy())
		{
			l = cast(l, l->getType());
			r = cast(r, l->getType());
		}
		else if (r->getType()->isPointerTy())
		{
			l = cast(l, r->getType());
			r = cast(r, r->getType());
		}

		Value* tmp = nullptr;
		if (operation.type==BinaryOperation::Lt)
		{
			tmp = floatOperation ? builder->CreateFCmp(CmpInst::FCMP_OLT, l, r, "lt") :
			      builder->CreateICmp(CmpInst::ICMP_SLT, l, r, "lt");
		}
		else if (operation.type==BinaryOperation::Le)
			tmp = floatOperation ? builder->CreateFCmp(CmpInst::FCMP_OLE, l, r, "le") :
			      builder->CreateICmp(CmpInst::ICMP_SLE, l, r, "le");
		else if (operation.type==BinaryOperation::Gt)
			tmp = floatOperation ? builder->CreateFCmp(CmpInst::FCMP_OGT, l, r, "gt") :
			      builder->CreateICmp(CmpInst::ICMP_SGT, l, r, "gt");
		else if (operation.type==BinaryOperation::Ge)
			tmp = floatOperation ? builder->CreateFCmp(CmpInst::FCMP_OGE, l, r, "ge") :
			      builder->CreateICmp(CmpInst::ICMP_SGE, l, r, "ge");
		else if (operation.type==BinaryOperation::Eq)
			tmp = floatOperation ? builder->CreateFCmp(CmpInst::FCMP_OEQ, l, r, "eq") :
			      builder->CreateICmp(CmpInst::ICMP_EQ, l, r, "eq");
		else if (operation.type==BinaryOperation::Neq)
			tmp = floatOperation ? builder->CreateFCmp(CmpInst::FCMP_UNE, l, r, "neq") :
			      builder->CreateICmp(CmpInst::ICMP_NE, l, r, "nq");
		else if (operation.type==BinaryOperation::And || operation.type==BinaryOperation::Or)
		{
			throw InternalError("logical operators are not yet supported in IR");
		}

		if (tmp) return builder->CreateZExt(tmp, builder->getInt32Ty());

		floatOperation = type().isFloatingType();
		l = cast(l, resultType);
		r = cast(r, resultType);
		if (operation.type==BinaryOperation::Mul)
			return builder->CreateBinOp(floatOperation ? Instruction::FMul : Instruction::Mul, l, r, "mul");
		else if (operation.type==BinaryOperation::Div)
			return builder->CreateBinOp(floatOperation ? Instruction::FDiv : Instruction::SDiv, l, r, "div");
		else if (operation.type==BinaryOperation::Mod) return builder->CreateBinOp(Instruction::SRem, l, r, "mod");
		else if (operation.type==BinaryOperation::Add)
			return builder->CreateBinOp(floatOperation ? Instruction::FAdd : Instruction::Add, l, r, "add");
		else if (operation.type==BinaryOperation::Sub)
			return builder->CreateBinOp(floatOperation ? Instruction::FSub : Instruction::Sub, l, r, "sub");
		throw InternalError("type is not supported in IR");
	}

	llvm::Value* increaseOrDecrease(bool inc, const ::Type& type, llvm::Value* input, const std::string& name)
	{
		llvm::Value* temp;
		std::string opName = inc ? "inc" : "dec";
		if (type.isFloatingType())
			temp = builder->CreateBinOp(inc ? Instruction::FAdd : Instruction::FSub, input,
					ConstantInt::get(builder->getFloatTy(), 1), opName);
		else if (type.isIntegerType())
			temp = builder->CreateBinOp(inc ? Instruction::Add : Instruction::Sub, input,
					builder->getInt32(1), opName);
		else if (type.isCharacterType())
			temp = builder->CreateBinOp(inc ? Instruction::Add : Instruction::Sub, input,
					builder->getInt8(1), opName);
		else if (type.isPointerType())
			temp = builder->CreateInBoundsGEP(input, builder->getInt32(inc*2-1), opName);
		else throw InternalError("type is not supported in IR");
		builder->CreateStore(temp, (*variables)[name]);
		return temp;
	}

	llvm::Value* PostfixExpr::codegen() const
	{
		auto result = variable->codegen();
		bool inc = operation.type==PostfixOperation::Incr;
		increaseOrDecrease(inc, type(), result, variable->name());
		return result;
	}

	llvm::Value* PrefixExpr::codegen() const
	{
		auto result = children()[0]->codegen();
		if (operation.type==PrefixOperation::Incr)
			return increaseOrDecrease(true, type(), result, children()[0]->name());
		else if (operation.type==PrefixOperation::Decr)
			return increaseOrDecrease(false, type(), result, children()[0]->name());
		else if (operation.type==PrefixOperation::Plus) return result;
		else if (operation.type==PrefixOperation::Neg)
		{
			auto type = result->getType();
			if (type->isFloatTy())
				return builder->CreateFSub(llvm::ConstantFP::get(type, 0), result, "neg");
			else if (type->isIntegerTy())
				return builder->CreateSub(llvm::ConstantInt::get(type, 0), result, "neg");
			else throw InternalError("type is not supported in IR");
		}
		else if (operation.type==PrefixOperation::Not)
		{
			throw InternalError("logical operators are not yet supported in IR");
		}
		else throw InternalError("type is not supported in IR");
	}

	llvm::Value* CastExpr::codegen() const
	{
		auto toCast = operand->codegen();
		auto to = type().convertToIR();
		return Ast::cast(toCast, to);
	}

	llvm::Value* Assignment::codegen() const
	{
		auto result = expr->codegen();
		auto tmp = cast(result, variable->type().convertToIR());
		builder->CreateStore(tmp, (*variables)[variable->name()]);
		return tmp;
	}

	llvm::Value* Declaration::codegen() const
	{
		auto type = variable->type().convertToIR();
		auto result = builder->CreateAlloca(type, nullptr, variable->name());
		(*variables)[variable->name()] = result;

		if (expr)
		{
			auto tmp = cast(expr->codegen(), type);
			builder->CreateStore(tmp, (*variables)[variable->name()]);
		}
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
			result = builder->CreateFPExt(result, builder->getDoubleTy());
			format = "%f\n";
			name = "floatFormat";
		}
		else if (expr->type().isIntegralType())
		{
			format = "%d\n";
			name = "intFormat";
		}
		else throw InternalError("type is not supported in IR");

		auto string = module->getNamedGlobal(name);
		if (!string)
		{
			return builder->CreateCall(module->getFunction("printf"),
					{builder->CreateGlobalStringPtr(format, name), result});
		}
		else
		{
			auto ptr = builder->CreateInBoundsGEP(string, {builder->getInt32(0), builder->getInt32(0)});
			return builder->CreateCall(module->getFunction("printf"), {ptr, result});
		}
	}
}
