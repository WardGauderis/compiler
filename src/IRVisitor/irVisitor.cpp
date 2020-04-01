//
// Created by ward on 3/28/20.
//

#include "irVisitor.h"

#include <llvm/IR/PassManager.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/IR/Verifier.h>
#include <ast/expressions.h>

using namespace llvm;

char RemoveUnusedCodeInBlockPass::ID = 0;

static RegisterPass<RemoveUnusedCodeInBlockPass> X("UnusedCode", "remove used code");

IRVisitor::IRVisitor(const std::filesystem::path& input)
		:module(input.string(), context), builder(context)
{
	module.getOrInsertFunction("printf",
			llvm::FunctionType::get(llvm::Type::getInt32PtrTy(context), builder.getInt8PtrTy(), true));
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
	ret = *variable.table->lookupAllocaInst(variable.name());
	if(!requiresLvalue) ret = builder.CreateLoad(ret);
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

		ret = cast(ret, builder.getInt1Ty());
		builder.CreateCondBr(ret, landTrue, landEnd);

		builder.SetInsertPoint(landTrue);
		binaryExpr.rhs->visit(*this);
		auto rhs = ret;
		const auto rhsType = rhs->getType();
		ret = cast(ret, builder.getInt1Ty());
		builder.CreateBr(landEnd);

		builder.SetInsertPoint(landEnd);
		auto phi = builder.CreatePHI(builder.getInt1Ty(), 2);
		phi->addIncoming(land ? builder.getFalse() : builder.getTrue(), current);
		phi->addIncoming(ret, landTrue);
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
	postFixExpr.operand->visit(*this);
	const auto toRet = ret;
	const auto& rhs = increaseOrDecrease(postFixExpr.operation.type==PostfixOperation::Incr, ret);
	requiresLvalue = true;
	postFixExpr.operand->visit(*this);
	requiresLvalue = false;
	builder.CreateStore(rhs, ret);
	ret = toRet;
}

void IRVisitor::visitPrefixExpr(const Ast::PrefixExpr& prefixExpr)
{
	const auto& opType = prefixExpr.operation.type;
	if (opType==PrefixOperation::Addr)
	{
		if (const auto& var = dynamic_cast<Ast::Variable*>(prefixExpr.operand))
		{
			requiresLvalue = true;
			var->visit(*this);
			requiresLvalue = false;
		}
		else if (const auto& deref = dynamic_cast<Ast::PrefixExpr*>(prefixExpr.operand))
		{
			deref->operand->visit(*this);
		}
		else throw IRError(prefixExpr.type().string());
		return;
	}
	else if(opType == PrefixOperation::Incr || opType == PrefixOperation::Decr)
	{
		prefixExpr.operand->visit(*this);
		const auto& rhs = increaseOrDecrease(opType==PrefixOperation::Incr, ret);
		requiresLvalue = true;
		prefixExpr.operand->visit(*this);
		requiresLvalue = false;
		builder.CreateStore(rhs, ret);
		ret = rhs;
		return;
	}

	prefixExpr.operand->visit(*this);
	const auto type = ret->getType();

	if (opType==PrefixOperation::Plus) return;
	else if (opType==PrefixOperation::Neg)
	{
		if (type->isFloatTy())
			ret = builder.CreateFSub(ConstantFP::get(type, 0), ret, "neg");
		else ret = builder.CreateSub(ConstantInt::get(type, 0), ret, "neg");
	}
	else if (opType==PrefixOperation::Not)
	{
		ret = cast(ret, builder.getInt1Ty());
		ret = builder.CreateXor(ret, builder.getTrue(), "not");
	}
	else if (opType==PrefixOperation::Deref)
	{
		ret = builder.CreateLoad(ret, "deref");
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
	assignment.rhs->visit(*this);
	auto rhs = cast(ret, convertToIR(assignment.lhs->type()));

	requiresLvalue = true;
	assignment.lhs->visit(*this);
	requiresLvalue = false;

	builder.CreateStore(rhs, ret);
}

void IRVisitor::visitDeclaration(const Ast::Declaration& declaration)
{
	const auto& ASTType = declaration.variable->type();
	const auto& type = convertToIR(ASTType);
	const auto& name = declaration.variable->name();
	auto& allocaInst = declaration.table->lookup(declaration.variable->name())->allocaInst;
	bool global = !declaration.table->getParent();
	if (global)
	{
		const auto& var = new GlobalVariable(module, type, ASTType.isConst(),
				GlobalValue::LinkageTypes::ExternalLinkage,
				Constant::getNullValue(type), name);
		allocaInst = var;
		if (declaration.expr)
		{
			declaration.expr->visit(*this);
			var->setInitializer(::cast<Constant>(ret));
		}
	}
	else
	{
		allocaInst = createAlloca(type, name);
		if (declaration.expr)
		{
			declaration.expr->visit(*this);
			ret = cast(ret, type);
			builder.CreateStore(ret, allocaInst);
		}
	}
}

void IRVisitor::visitPrintfStatement(const Ast::PrintfStatement& printfStatement)
{
	printfStatement.expr->visit(*this);
	const auto type = ret->getType();

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
	ifStatement.condition->visit(*this);
	const auto type = ret->getType();
	const auto& ifTrue = BasicBlock::Create(context, "if.true", builder.GetInsertBlock()->getParent());
	BasicBlock* ifFalse = (ifStatement.elseBody==nullptr) ? nullptr : BasicBlock::Create(context, "if.false",
			builder.GetInsertBlock()->getParent());
	const auto& ifEnd = BasicBlock::Create(context, "if.end", builder.GetInsertBlock()->getParent());

	ret = cast(ret, builder.getInt1Ty());
	builder.CreateCondBr(ret, ifTrue, ifFalse ? ifFalse : ifEnd);

	builder.SetInsertPoint(ifTrue);
	ifStatement.ifBody->visit(*this);
	builder.CreateBr(ifEnd);

	if (ifFalse)
	{
		builder.SetInsertPoint(ifFalse);
		ifStatement.elseBody->visit(*this);
		builder.CreateBr(ifEnd);
	}

	builder.SetInsertPoint(ifEnd);
}

void IRVisitor::visitLoopStatement(const Ast::LoopStatement& loopStatement)
{
	const auto& loopCond = BasicBlock::Create(context, "loop.cond", builder.GetInsertBlock()->getParent());
	const auto& loopBody = BasicBlock::Create(context, "loop.body", builder.GetInsertBlock()->getParent());
	const auto& loopEnd = BasicBlock::Create(context, "loop.end", builder.GetInsertBlock()->getParent());
	const auto& loopIter = loopStatement.iteration ? BasicBlock::Create(context, "loop.iter",
			builder.GetInsertBlock()->getParent()) : nullptr;

	if (loopStatement.init) loopStatement.init->visit(*this);
	builder.CreateBr(loopCond);

	builder.SetInsertPoint(loopCond);
	if (loopStatement.condition)
	{
		loopStatement.condition->visit(*this);
		const auto type = ret->getType();
		ret = cast(ret, builder.getInt1Ty());
		builder.CreateCondBr(ret, loopBody, loopEnd);
	}
	else builder.CreateBr(loopBody);

	builder.SetInsertPoint(loopBody);
	const auto& breakBackup = breakBlock;
	const auto& continueBackup = continueBlock; //TODO
	breakBlock = loopEnd;
	continueBlock = loopIter ? loopIter : loopCond;
	loopStatement.body->visit(*this);
	breakBlock = breakBackup;
	continueBlock = continueBackup;
	builder.CreateBr(loopIter ? loopIter : loopCond);

	if (loopIter)
	{
		builder.SetInsertPoint(loopIter);
		loopStatement.iteration->visit(*this);
		builder.CreateBr(loopCond);
	}

	builder.SetInsertPoint(loopEnd);
}

void IRVisitor::visitControlStatement(const Ast::ControlStatement& controlStatement)
{
	if (controlStatement.name()=="break") builder.CreateBr(breakBlock);
	else builder.CreateBr(continueBlock);
}

void IRVisitor::visitReturnStatement(const Ast::ReturnStatement& returnStatement)
{
	returnStatement.expr->visit(*this);
	ret = cast(ret, builder.getCurrentFunctionReturnType());
	builder.CreateRet(ret);
}

void IRVisitor::visitFunctionDefinition(const Ast::FunctionDefinition& functionDefinition)
{
	std::vector<llvm::Type*> parameters;
	for (const auto& parameter: functionDefinition.parameters)
	{
		parameters.emplace_back(convertToIR(parameter.first));
	}
	const auto& returnType = convertToIR(functionDefinition.returnType, true);
	const auto& functionType = llvm::FunctionType::get(returnType, parameters, false);
	const auto function =
			::cast<Function>(module.getOrInsertFunction(functionDefinition.identifier, functionType).getCallee());
	functionDefinition.table->lookup(functionDefinition.identifier)->allocaInst = function;
	const auto& block = BasicBlock::Create(context, "entry", function);
	builder.SetInsertPoint(block);
	size_t i = 0;
	for (auto& parameter: function->args())
	{
		const auto& name = functionDefinition.parameters[i++].second;
		ret = createAlloca(parameter.getType(), name);
		functionDefinition.body->table->lookup(name)->allocaInst = ret;
		builder.CreateStore(&parameter, ret);
	}
	functionDefinition.body->visit(*this);
	if (returnType->isVoidTy()) builder.CreateRetVoid();
	else
		builder.CreateRet(functionDefinition.identifier=="main" ? Constant::getNullValue(returnType) :
		                  UndefValue::get(returnType));
	removeUnusedCode.runOnFunction(*function);
}

void IRVisitor::visitFunctionCall(const Ast::FunctionCall& functionCall)
{
	const auto& function = *functionCall.table->lookupAllocaInst(functionCall.value());
	std::vector<Value*> arguments;
	for (const auto& argument: functionCall.arguments)
	{
		argument->visit(*this);
		arguments.emplace_back(ret);
	}
	ret = builder.CreateCall(function, arguments);
}

llvm::Value* IRVisitor::cast(llvm::Value* value, llvm::Type* to)
{
	auto from = value->getType();
	if (from==to) return value;

	if (from==builder.getInt1Ty())
	{
		if (to->isIntegerTy()) return builder.CreateZExt(value, to);
		else if (to->isFloatTy()) return builder.CreateFPToUI(value, to);
		else if (to->isPointerTy()) return builder.CreateIntToPtr(value, to);
	}
	else if (from->isIntegerTy())
	{
		if (to==builder.getInt1Ty()) return builder.CreateICmpNE(value, Constant::getNullValue(from));
		else if (to->isIntegerTy()) return builder.CreateSExtOrTrunc(value, to);
		else if (to->isFloatTy()) return builder.CreateSIToFP(value, to);
		else if (to->isPointerTy()) return builder.CreateIntToPtr(value, to);
	}
	else if (from->isFloatTy())
	{
		if (to==builder.getInt1Ty()) return builder.CreateFCmpUNE(value, Constant::getNullValue(from));
		else if (to->isIntegerTy()) return builder.CreateFPToSI(value, to);
	}
	else if (from->isPointerTy())
	{
		if (to==builder.getInt1Ty()) return builder.CreateICmpNE(value, Constant::getNullValue(from));
		else if (to->isIntegerTy()) return builder.CreatePtrToInt(value, to);
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

llvm::Type* IRVisitor::convertToIR(const ::Type& type, const bool function)
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
		return PointerType::getUnqual(convertToIR(type.getDerefType().value()));
	}
	else if (type.isVoidType())
	{
		if (function) return builder.getVoidTy();
		return builder.getInt8Ty();
	}
	throw IRError(type.string());
}

llvm::AllocaInst* IRVisitor::createAlloca(llvm::Type* type, const std::string& name)
{
	auto& block = builder.GetInsertBlock()->getParent()->getEntryBlock();
	IRBuilder tmpBuilder(&block, block.begin());
	return tmpBuilder.CreateAlloca(type, nullptr, name);
}
