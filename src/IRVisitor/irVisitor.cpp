//
// Created by ward on 3/28/20.
//

#include "irVisitor.h"

#include <llvm/IR/PassManager.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/IR/Verifier.h>
#include <ast/expressions.h>

using namespace llvm;

IRVisitor::IRVisitor(const std::filesystem::path& input)
		:module(input.string(), context), builder(context)
{
	module.getOrInsertFunction("printf",
			llvm::FunctionType::get(llvm::Type::getInt32PtrTy(context), builder.getInt8PtrTy(), true));

	FunctionCallee tmp = module.getOrInsertFunction("main", builder.getInt32Ty());
	auto main = ::cast<Function>(tmp.getCallee());

	auto block = BasicBlock::Create(context, "entry", main);
	builder.SetInsertPoint(block);
}

void IRVisitor::convertAST(const std::unique_ptr<Ast::Node>& root)
{
	root->visit(*this);
	verifyModule(module, &errs());
}

void IRVisitor::LLVMOptimize()
{
	PassBuilder passBuilder;
	LoopAnalysisManager loopAnalysisManager(true);
	FunctionAnalysisManager functionAnalysisManager(true);
	CGSCCAnalysisManager cGSCCAnalysisManager(true);
	ModuleAnalysisManager moduleAnalysisManager(true);
	passBuilder.registerModuleAnalyses(moduleAnalysisManager);
	passBuilder.registerCGSCCAnalyses(cGSCCAnalysisManager);
	passBuilder.registerFunctionAnalyses(functionAnalysisManager);
	passBuilder.registerLoopAnalyses(loopAnalysisManager);
	passBuilder.crossRegisterProxies(loopAnalysisManager, functionAnalysisManager, cGSCCAnalysisManager,
			moduleAnalysisManager);
	ModulePassManager modulePassManager = passBuilder.buildPerModuleDefaultPipeline(
			PassBuilder::OptimizationLevel::O3);
	modulePassManager.run(module, moduleAnalysisManager);
}

void IRVisitor::print(const std::filesystem::path& output)
{
	std::error_code ec;
	raw_fd_ostream out(output.string(), ec);
	module.print(out, nullptr, false, true);
}

void IRVisitor::visitLiteral(const Ast::Literal& literal)
{
	auto type = literal.type();
	if (type.isCharacterType())
		ret = ConstantInt::get(builder.getInt8Ty(), std::get<char>(literal.literal));
	else if (type.isIntegerType())
		ret = ConstantInt::get(builder.getInt32Ty(), std::get<int>(literal.literal));
	else if (type.isFloatType())
		ret = ConstantFP::get(builder.getFloatTy(), std::get<float>(literal.literal));
	else throw IRError(type.string());
}

void IRVisitor::visitComment(const Ast::Comment& comment)
{
	MDNode* m = MDNode::get(context, MDString::get(context, comment.value()));
}

void IRVisitor::visitVariable(const Ast::Variable& variable)
{
	ret = builder.CreateLoad(variables[variable.name()]);
}

void IRVisitor::visitScope(const Ast::Scope& scope)
{
	for (const auto& statement: scope.children())
	{
		statement->visit(*this);
	}
}

void IRVisitor::visitBinaryExpr(const Ast::BinaryExpr& binaryExpr)
{
	binaryExpr.lhs->visit(*this);
	auto lhs = ret;
	ret = nullptr;

	const auto targetType = convertToIR(binaryExpr.type());
	const auto& operationType = binaryExpr.operation.type;
	bool floatOperation = false;
	bool pointerOperation = false;
	const auto lhsType = lhs->getType();

	if (binaryExpr.operation.isLogicalOperator())
	{
		bool land = operationType==BinaryOperation::And;
		auto landTrue = BasicBlock::Create(context, land ? "land.true" : "lor.false",
				builder.GetInsertBlock()->getParent());
		auto landEnd = BasicBlock::Create(context, land ? "land.end" : "lor.end",
				builder.GetInsertBlock()->getParent());
		auto current = builder.GetInsertBlock();

		ret = lhsType->isFloatTy() ? builder.CreateFCmp(CmpInst::FCMP_UNE, lhs, Constant::getNullValue(lhsType)) :
		      builder.CreateICmp(CmpInst::ICMP_NE, lhs, Constant::getNullValue(lhsType));

		builder.CreateCondBr(ret, landTrue, landEnd);

		builder.SetInsertPoint(landTrue);
		binaryExpr.rhs->visit(*this);
		auto rhs = ret;
		const auto rhsType = rhs->getType();
		ret = rhsType->isFloatTy() ? builder.CreateFCmp(CmpInst::FCMP_UNE, rhs, Constant::getNullValue(rhsType)) :
		      builder.CreateICmp(CmpInst::ICMP_NE, rhs, Constant::getNullValue(rhsType));
		builder.CreateBr(landEnd);

		builder.SetInsertPoint(landEnd);
		auto phi = builder.CreatePHI(builder.getInt1Ty(), 2);
		phi->addIncoming(land ? builder.getFalse() : builder.getTrue(), current);
		phi->addIncoming(ret, landTrue);
		ret = builder.CreateZExt(ret, builder.getInt32Ty());
		return;
	}

	binaryExpr.rhs->visit(*this);
	auto rhs = ret;
	const auto rhsType = rhs->getType();

	if (binaryExpr.operation.isComparisonOperator())
	{
		if (lhsType->isFloatTy() || rhsType->isFloatTy())
		{
			lhs = cast(lhs, builder.getFloatTy());
			rhs = cast(rhs, builder.getFloatTy());
			floatOperation = true;
		}
		else if (lhsType->isPointerTy())
		{
			rhs = cast(rhs, lhs->getType());
			pointerOperation = true;
		}
		else if (rhsType->isPointerTy())
		{
			lhs = cast(lhs, rhs->getType());
			pointerOperation = true;
		}
		else if (lhsType->isIntegerTy(32) || rhsType->isIntegerTy(32))
		{
			lhs = cast(lhs, builder.getInt32Ty());
			rhs = cast(rhs, builder.getInt32Ty());
		}

		if (operationType==BinaryOperation::Lt)
			ret = floatOperation ? builder.CreateFCmp(CmpInst::FCMP_OLT, lhs, rhs, "lt") :
			      builder.CreateICmp(pointerOperation ? CmpInst::ICMP_ULT : CmpInst::ICMP_SLT, lhs, rhs, "lt");
		else if (operationType==BinaryOperation::Le)
			ret = floatOperation ? builder.CreateFCmp(CmpInst::FCMP_OLE, lhs, rhs, "le") :
			      builder.CreateICmp(pointerOperation ? CmpInst::ICMP_ULE : CmpInst::ICMP_SLE, lhs, rhs, "le");
		else if (operationType==BinaryOperation::Gt)
			ret = floatOperation ? builder.CreateFCmp(CmpInst::FCMP_OGT, lhs, rhs, "gt") :
			      builder.CreateICmp(pointerOperation ? CmpInst::ICMP_UGT : CmpInst::ICMP_SGT, lhs, rhs, "gt");
		else if (operationType==BinaryOperation::Ge)
			ret = floatOperation ? builder.CreateFCmp(CmpInst::FCMP_OGE, lhs, rhs, "ge") :
			      builder.CreateICmp(pointerOperation ? CmpInst::ICMP_UGE : CmpInst::ICMP_SGE, lhs, rhs, "ge");
		else if (operationType==BinaryOperation::Eq)
			ret = floatOperation ? builder.CreateFCmp(CmpInst::FCMP_OEQ, lhs, rhs, "eq") :
			      builder.CreateICmp(CmpInst::ICMP_EQ, lhs, rhs, "eq");
		else if (operationType==BinaryOperation::Neq)
			ret = floatOperation ? builder.CreateFCmp(CmpInst::FCMP_UNE, lhs, rhs, "neq") :
			      builder.CreateICmp(CmpInst::ICMP_NE, lhs, rhs, "nq");

		ret = builder.CreateZExt(ret, builder.getInt32Ty());
	}
	else if (operationType==BinaryOperation::Add && lhs->getType()->isPointerTy())
		ret = builder.CreateInBoundsGEP(lhs, rhs);
	else if (operationType==BinaryOperation::Add && rhs->getType()->isPointerTy())
		ret = builder.CreateInBoundsGEP(rhs, lhs);
	else if (operationType==BinaryOperation::Sub && lhs->getType()->isPointerTy())
	{
		ret = builder.CreateSub(ConstantInt::get(rhs->getType(), 0), rhs);
		ret = builder.CreateInBoundsGEP(lhs, ret);
	}
	else
	{
		floatOperation = targetType->isFloatTy();
		lhs = cast(lhs, targetType);
		rhs = cast(rhs, targetType);
		if (operationType==BinaryOperation::Add)
			ret = builder.CreateBinOp(floatOperation ? Instruction::FAdd : Instruction::Add, lhs, rhs, "add");
		else if (operationType==BinaryOperation::Sub)
			ret = builder.CreateBinOp(floatOperation ? Instruction::FSub : Instruction::Sub, lhs, rhs, "sub");
		else if (operationType==BinaryOperation::Mul)
			ret = builder.CreateBinOp(floatOperation ? Instruction::FMul : Instruction::Mul, lhs, rhs, "mul");
		else if (operationType==BinaryOperation::Div)
			ret = builder.CreateBinOp(floatOperation ? Instruction::FDiv : Instruction::SDiv, lhs, rhs, "div");
		else if (operationType==BinaryOperation::Mod)
			ret = builder.CreateBinOp(Instruction::SRem, lhs, rhs, "mod");
	}

	if (!ret) throw InternalError("Encountered binary operation is not supported in LLVM IR");
}

void IRVisitor::visitPostfixExpr(const Ast::PostfixExpr& postFixExpr)
{
	postFixExpr.variable->visit(*this);
	bool inc = postFixExpr.operation.type==PostfixOperation::Incr;
	auto temp = increaseOrDecrease(inc, ret);
	builder.CreateStore(temp, variables[postFixExpr.variable->name()]);
}

void IRVisitor::visitPrefixExpr(const Ast::PrefixExpr& prefixExpr)
{
	prefixExpr.children()[0]->visit(*this);
	const auto& optType = prefixExpr.operation.type;
	const auto& type = ret->getType();

	if (optType==PrefixOperation::Plus) return;
	else if (optType==PrefixOperation::Neg)
	{
		if (type->isFloatTy())
			ret = builder.CreateFSub(ConstantFP::get(type, 0), ret, "neg");
		else ret = builder.CreateSub(ConstantInt::get(type, 0), ret, "neg");
	}
	else if (optType==PrefixOperation::Not)
	{
		ret = type->isFloatTy() ? builder.CreateFCmp(CmpInst::FCMP_UNE, ret, Constant::getNullValue(type)) :
		      builder.CreateICmp(CmpInst::ICMP_NE, ret, Constant::getNullValue(type));
		ret = builder.CreateXor(ret, builder.getTrue());
		ret = builder.CreateZExt(ret, builder.getInt32Ty());
	}
	else
	{
		ret = increaseOrDecrease(optType==PrefixOperation::Incr, ret);
		builder.CreateStore(ret, variables[prefixExpr.children()[0]->name()]);
	}
}

void IRVisitor::visitCastExpr(const Ast::CastExpr& castExpr)
{
	castExpr.operand->visit(*this);
	auto to = convertToIR(castExpr.type());
	ret = cast(ret, to);
}

void IRVisitor::visitAssignment(const Ast::Assignment& assignment)
{
	assignment.expr->visit(*this);
	ret = cast(ret, convertToIR(assignment.variable->type()));
	builder.CreateStore(ret, variables[assignment.variable->name()]);
}

void IRVisitor::visitDeclaration(const Ast::Declaration& declaration)
{
	const auto& type = convertToIR(declaration.variable->type());
	const auto& name = declaration.variable->name();
	variables[name] = builder.CreateAlloca(type, nullptr, name);

	if (declaration.expr)
	{
		declaration.expr->visit(*this);
		ret = cast(ret, type);
		builder.CreateStore(ret, variables[name]);
	}
}

void IRVisitor::visitPrintfStatement(const Ast::PrintfStatement& printfStatement)
{
	printfStatement.expr->visit(*this);
	const auto& type = ret->getType();

	std::string format;
	std::string name;
	if (type->isPointerTy())
	{
		format = "%p\n";
		name = "ptrFormat";
	}
	else if (type->isFloatTy())
	{
		ret = builder.CreateFPExt(ret, builder.getDoubleTy());
		format = "%f\n";
		name = "floatFormat";
	}
	else
	{
		format = "%d\n";
		name = "intFormat";
	}

	auto string = module.getNamedGlobal(name);
	if (!string)
		ret = builder.CreateCall(module.getFunction("printf"), {builder.CreateGlobalStringPtr(format, name), ret});
	else
		ret = builder.CreateCall(module.getFunction("printf"),
				{builder.CreateInBoundsGEP(string, {builder.getInt32(0), builder.getInt32(0)}), ret});
}

void IRVisitor::visitIfStatement(const Ast::IfStatement& ifStatement)
{
	throw InternalError("If statement not yet supported in LLVM IR");
}

void IRVisitor::visitLoopStatement(const Ast::LoopStatement& loopStatement)
{
	throw InternalError("Loop statement not yet supported in LLVM IR");
}

void IRVisitor::visitControlStatement(const Ast::ControlStatement& controlStatement)
{
	throw InternalError("Control statement not yet supported in LLVM IR");
}

void IRVisitor::visitReturnStatement(const Ast::ReturnStatement& returnStatement)
{
	throw InternalError("Return statement not yet supported in LLVM IR");
}

void IRVisitor::visitFunctionDefinition(const Ast::FunctionDefinition& functionDefinition)
{
	throw InternalError("Function definition not yet supported in LLVM IR");
}

void IRVisitor::visitFunctionCall(const Ast::FunctionCall& functionCall)
{
	throw InternalError("Function call not yet supported in LLVM IR");
}

llvm::Value* IRVisitor::cast(llvm::Value* value, llvm::Type* to)
{
	auto from = value->getType();
	if (from==to) return value;

	if (from->isIntegerTy())
	{
		if (to->isIntegerTy()) return builder.CreateSExtOrTrunc(value, to);
		else if (to->isFloatTy()) return builder.CreateSIToFP(value, to);
		else if (to->isPointerTy()) return builder.CreateIntToPtr(value, to);
	}
	else if (from->isFloatTy())
	{
		if (to->isIntegerTy()) return builder.CreateFPToSI(value, to);
	}
	else if (from->isPointerTy())
	{
		if (to->isIntegerTy()) return builder.CreatePtrToInt(value, to);
		else if (to->isPointerTy()) return builder.CreatePointerCast(value, to);
	}

	throw InternalError("Invalid cast expression in LLVM IR");
}

llvm::Value* IRVisitor::increaseOrDecrease(const bool inc, llvm::Value* input)
{
	auto type = input->getType();
	std::string opName = inc ? "inc" : "dec";
	if (type->isFloatTy())
		return builder.CreateBinOp(inc ? Instruction::FAdd : Instruction::FSub, input,
				ConstantFP::get(builder.getFloatTy(), 1), opName);
	else if (type->isPointerTy()) return builder.CreateInBoundsGEP(input, builder.getInt32(inc*2-1), opName);
	else
		return builder.CreateBinOp(inc ? Instruction::Add : Instruction::Sub, input, ConstantInt::get(type, 1), opName);
}

llvm::Type* IRVisitor::convertToIR(const ::Type& type)
{
	if (type.isBaseType())
	{
		switch (type.getBaseType())
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
	else if (type.isPointerType())
	{
		return llvm::PointerType::getUnqual(convertToIR(type.getDerefType().value()));
	}
	throw IRError(type.string());
}
