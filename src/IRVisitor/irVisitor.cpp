//
// Created by ward on 3/28/20.
//

#include "irVisitor.h"

#include <ast/expressions.h>
#include <llvm/IR/PassManager.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Transforms/IPO.h>
#include "MIPSVisitor/mipsVisitor.h"
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Utils.h>
#include "llvmPasses.h"

using namespace llvm;

char RemoveUnusedCodeInBlockPass::ID = 0;
char RemovePhiInstructionPass::ID = 0;

//static RegisterPass<RemoveUnusedCodeInBlockPass> X("UnusedCode", "remove unused code");
//static RegisterPass<RemoveUnusedCodeInBlockPass> Y("RemovePhi", "remove phi instructions");

IRVisitor::IRVisitor(const std::filesystem::path& input)
		:module(input.string(), context), builder(context)
{
	module.setDataLayout("p:32:32");
}

void IRVisitor::convertAST(const std::unique_ptr<Ast::Node>& root)
{
	root->visit(*this);
	verifyModule(module, &errs());
}

void IRVisitor::LLVMOptimize(const int level)
{
	if (level==1) {
		createConstantMergePass()->runOnModule(module);
		legacy::FunctionPassManager m(&module);
		m.add(createPromoteMemoryToRegisterPass());
		m.add(createSROAPass());

		for (auto& function: module.functions()) {
			m.run(function);
		}
	}
	else if (level>=2) {
		std::cout << CompilationError(
				"Optimisation level 2 may not work in MIPS because it may introduce unsupported LLVM IR instructions",
				0, 0, true);
		PassBuilder passBuilder;
		LoopAnalysisManager loopAnalysisManager(false);
		FunctionAnalysisManager functionAnalysisManager(false);
		CGSCCAnalysisManager cGSCCAnalysisManager(false);
		ModuleAnalysisManager moduleAnalysisManager(false);
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
	RemovePhiInstructionPass pass;
	for (auto& F: module) {
		pass.runOnFunction(F);
	}
}

void IRVisitor::print(const std::filesystem::path& output)
{
	std::error_code ec;
	raw_fd_ostream out(output.string(), ec);
	module.setDataLayout("");
	module.print(out, nullptr, false, true);
	module.setDataLayout("p:32:32");
}

void IRVisitor::visitLiteral(const Ast::Literal& literal)
{
	auto type = literal.type();
	if (type->isCharacterType())
		ret = ConstantInt::get(builder.getInt8Ty(), std::get<char>(literal.literal));
	else if (type->isIntegerType())
		ret = ConstantInt::get(builder.getInt32Ty(), std::get<int>(literal.literal));
	else if (type->isFloatType()) ret = ConstantFP::get(builder.getFloatTy(), std::get<float>(literal.literal));
	else throw IRError(type->string());
}

void IRVisitor::visitStringLiteral(const Ast::StringLiteral& stringLiteral)
{
	Constant* StrConstant = ConstantDataArray::getString(context, stringLiteral.value());
	auto* GV = new GlobalVariable(module, StrConstant->getType(), true,
			GlobalValue::PrivateLinkage, StrConstant, "",
			nullptr, GlobalVariable::NotThreadLocal, 0);
	GV->setUnnamedAddr(GlobalValue::UnnamedAddr::Global);
	GV->setAlignment(Align(1));
	ret = GV;
}

void IRVisitor::visitComment(const Ast::Comment& comment)
{
	MDNode::get(context, MDString::get(context, comment.value()));
}

void IRVisitor::visitVariable(const Ast::Variable& variable)
{
	const auto tmp = variable.table->lookupAllocaInst(variable.name());
	if (!tmp) throw InternalError("'"+variable.name()+"' undeclared in LLVM IR");
	ret = *tmp;
	isRvalue = false;
}

void IRVisitor::visitScope(const Ast::Scope& scope)
{
	for (const auto& statement : scope.children()) {
		statement->visit(*this);
	}
}

void IRVisitor::visitBinaryExpr(const Ast::BinaryExpr& binaryExpr)
{
	auto lhs = LRValue(binaryExpr.lhs, true);
	ret = nullptr;

	const auto targetType = convertToIR(binaryExpr.type());
	const auto& operationType = binaryExpr.operation.type;
	bool floatOperation = false;
	bool pointerOperation = false;
	const auto lhsType = lhs->getType();

	if (binaryExpr.operation.isLogicalOperator()) {
		bool lAnd = operationType==BinaryOperation::And;
		auto lRhs = BasicBlock::Create(context, lAnd ? "land.true" : "lor.false",
				builder.GetInsertBlock()->getParent());
		auto lEnd = BasicBlock::Create(context, lAnd ? "land.end" : "lor.end",
				builder.GetInsertBlock()->getParent());
		auto current = builder.GetInsertBlock();

		lhs = cast(lhs, builder.getInt1Ty());
		if (lAnd)
			builder.CreateCondBr(lhs, lRhs, lEnd);
		else
			builder.CreateCondBr(lhs, lEnd, lRhs);

		builder.SetInsertPoint(lRhs);
		auto rhs = LRValue(binaryExpr.rhs, true);
		rhs = cast(rhs, builder.getInt1Ty());
		builder.CreateBr(lEnd);

		builder.SetInsertPoint(lEnd);
		auto phi = builder.CreatePHI(builder.getInt1Ty(), 2);
		phi->addIncoming(lAnd ? builder.getFalse() : builder.getTrue(), current);
		phi->addIncoming(rhs, lRhs);
		ret = phi;
		return;
	}

	auto rhs = LRValue(binaryExpr.rhs, true);
	const auto rhsType = rhs->getType();

	if (binaryExpr.operation.isComparisonOperator()) {
		if (lhsType->isFloatTy() || rhsType->isFloatTy()) {
			lhs = cast(lhs, builder.getFloatTy());
			rhs = cast(rhs, builder.getFloatTy());
			floatOperation = true;
		}
		else if (lhsType->isPointerTy()) {
			rhs = cast(rhs, lhs->getType());
			pointerOperation = true;
		}
		else if (rhsType->isPointerTy()) {
			lhs = cast(lhs, rhs->getType());
			pointerOperation = true;
		}
		else if (lhsType->isIntegerTy(32) || rhsType->isIntegerTy(32)) {
			lhs = cast(lhs, builder.getInt32Ty());
			rhs = cast(rhs, builder.getInt32Ty());
		}

		if (operationType==BinaryOperation::Lt)
			ret = floatOperation
			      ? builder.CreateFCmp(CmpInst::FCMP_OLT, lhs, rhs, "lt")
			      : builder.CreateICmp(pointerOperation ? CmpInst::ICMP_ULT
			                                            : CmpInst::ICMP_SLT,
							lhs, rhs, "lt");
		else if (operationType==BinaryOperation::Le)
			ret = floatOperation
			      ? builder.CreateFCmp(CmpInst::FCMP_OLE, lhs, rhs, "le")
			      : builder.CreateICmp(pointerOperation ? CmpInst::ICMP_ULE
			                                            : CmpInst::ICMP_SLE,
							lhs, rhs, "le");
		else if (operationType==BinaryOperation::Gt)
			ret = floatOperation
			      ? builder.CreateFCmp(CmpInst::FCMP_OGT, lhs, rhs, "gt")
			      : builder.CreateICmp(pointerOperation ? CmpInst::ICMP_UGT
			                                            : CmpInst::ICMP_SGT,
							lhs, rhs, "gt");
		else if (operationType==BinaryOperation::Ge)
			ret = floatOperation
			      ? builder.CreateFCmp(CmpInst::FCMP_OGE, lhs, rhs, "ge")
			      : builder.CreateICmp(pointerOperation ? CmpInst::ICMP_UGE
			                                            : CmpInst::ICMP_SGE,
							lhs, rhs, "ge");
		else if (operationType==BinaryOperation::Eq)
			ret = floatOperation
			      ? builder.CreateFCmp(CmpInst::FCMP_OEQ, lhs, rhs, "eq")
			      : builder.CreateICmp(CmpInst::ICMP_EQ, lhs, rhs, "eq");
		else if (operationType==BinaryOperation::Neq)
			ret = floatOperation
			      ? builder.CreateFCmp(CmpInst::FCMP_UNE, lhs, rhs, "neq")
			      : builder.CreateICmp(CmpInst::ICMP_NE, lhs, rhs, "nq");
	}
	else if (operationType==BinaryOperation::Add &&
			lhs->getType()->isPointerTy())
		ret = builder.CreateInBoundsGEP(lhs, rhs);
	else if (operationType==BinaryOperation::Add &&
			rhs->getType()->isPointerTy())
		ret = builder.CreateInBoundsGEP(rhs, lhs);
	else if (operationType==BinaryOperation::Sub &&
			lhs->getType()->isPointerTy()) {
		ret = builder.CreateSub(ConstantInt::get(rhs->getType(), 0), rhs);
		ret = builder.CreateInBoundsGEP(lhs, ret);
	}
	else {
		floatOperation = targetType->isFloatTy();
		lhs = cast(lhs, targetType);
		rhs = cast(rhs, targetType);
		if (operationType==BinaryOperation::Add)
			ret = builder.CreateBinOp(floatOperation ? Instruction::FAdd
			                                         : Instruction::Add,
					lhs, rhs, "add");
		else if (operationType==BinaryOperation::Sub)
			ret = builder.CreateBinOp(floatOperation ? Instruction::FSub
			                                         : Instruction::Sub,
					lhs, rhs, "sub");
		else if (operationType==BinaryOperation::Mul)
			ret = builder.CreateBinOp(floatOperation ? Instruction::FMul
			                                         : Instruction::Mul,
					lhs, rhs, "mul");
		else if (operationType==BinaryOperation::Div)
			ret = builder.CreateBinOp(floatOperation ? Instruction::FDiv
			                                         : Instruction::SDiv,
					lhs, rhs, "div");
		else if (operationType==BinaryOperation::Mod)
			ret = builder.CreateBinOp(Instruction::SRem, lhs, rhs, "mod");
	}

	if (!ret)
		throw InternalError(
				"Encountered binary operation is not supported in LLVM IR");
}

void IRVisitor::visitPostfixExpr(const Ast::PostfixExpr& postFixExpr)
{
	const auto lvalue = LRValue(postFixExpr.operand, false);
	const auto rvalue = builder.CreateLoad(lvalue);

	const auto& rhs = increaseOrDecrease(
			postFixExpr.operation.type==PostfixOperation::Incr, rvalue);
	builder.CreateStore(rhs, lvalue);

	ret = rvalue;
}

void IRVisitor::visitPrefixExpr(const Ast::PrefixExpr& prefixExpr)
{
	const auto& opType = prefixExpr.operation.type;
	if (opType==PrefixOperation::Addr) {
		ret = LRValue(prefixExpr.operand, false);
		return;
	}
	else if (opType==PrefixOperation::Incr ||
			opType==PrefixOperation::Decr) {
		const auto lvalue = LRValue(prefixExpr.operand, false);
		const auto rvalue = builder.CreateLoad(lvalue);

		ret = increaseOrDecrease(opType==PrefixOperation::Incr, rvalue);
		builder.CreateStore(ret, lvalue);
		return;
	}

	ret = LRValue(prefixExpr.operand, true);
	const auto type = ret->getType();

	if (opType==PrefixOperation::Plus)
		return;
	else if (opType==PrefixOperation::Neg) {
		if (type->isFloatTy())
			ret = builder.CreateFSub(ConstantFP::get(type, 0), ret, "neg");
		else
			ret = builder.CreateSub(ConstantInt::get(type, 0), ret, "neg");
	}
	else if (opType==PrefixOperation::Not) {
		ret = cast(ret, builder.getInt1Ty());
		ret = builder.CreateXor(ret, builder.getTrue(), "not");
	}
	else if (opType==PrefixOperation::Deref) {
		isRvalue = false;
	}
}

void IRVisitor::visitCastExpr(const Ast::CastExpr& castExpr)
{
	ret = LRValue(castExpr.operand, true);
	auto to = convertToIR(castExpr.type());
	ret = cast(ret, to);
}

void IRVisitor::visitAssignment(const Ast::Assignment& assignment)
{
	auto rhs = LRValue(assignment.rhs, true);
	auto lhs = LRValue(assignment.lhs, false);
	rhs = cast(rhs, lhs->getType()->getContainedType(0));
	builder.CreateStore(rhs, lhs);
	ret = rhs;
}

void IRVisitor::visitDeclaration(const Ast::VariableDeclaration& declaration)
{
	const auto& ASTType = declaration.type;
	const auto& type = convertToIR(ASTType);
	const auto& name = declaration.identifier;
	auto& allocaInst = declaration.table->lookup(name)->allocaInst;
	bool global = !declaration.table->getParent();
	if (global) {
		const auto& var =
				new GlobalVariable(module, type, ASTType->isConst(),
						GlobalValue::LinkageTypes::ExternalLinkage,
						Constant::getNullValue(type), name);
		allocaInst = var;
		if (declaration.expr) {
			ret = LRValue(declaration.expr, true);
			ret = cast(ret, type);
			var->setInitializer(llvm::cast<Constant>(ret));
		}
	}
	else {
		allocaInst = createAlloca(type, name);
		if (declaration.expr) {
			ret = LRValue(declaration.expr, true);
			ret = cast(ret, type);
			builder.CreateStore(ret, allocaInst);
		}
	}
}

void IRVisitor::visitIfStatement(const Ast::IfStatement& ifStatement)
{
	ret = LRValue(ifStatement.condition, true);
	const auto& ifTrue = BasicBlock::Create(
			context, "if.true", builder.GetInsertBlock()->getParent());
	BasicBlock* ifFalse =
			(ifStatement.elseBody==nullptr)
			? nullptr
			: BasicBlock::Create(context, "if.false",
					builder.GetInsertBlock()->getParent());
	const auto& ifEnd = BasicBlock::Create(context, "if.end",
			builder.GetInsertBlock()->getParent());

	ret = cast(ret, builder.getInt1Ty());
	builder.CreateCondBr(ret, ifTrue, ifFalse ? ifFalse : ifEnd);

	builder.SetInsertPoint(ifTrue);
	ifStatement.ifBody->visit(*this);
	builder.CreateBr(ifEnd);

	if (ifFalse) {
		builder.SetInsertPoint(ifFalse);
		ifStatement.elseBody->visit(*this);
		builder.CreateBr(ifEnd);
	}

	builder.SetInsertPoint(ifEnd);
}

void IRVisitor::visitLoopStatement(const Ast::LoopStatement& loopStatement)
{
	const auto& loopCond = BasicBlock::Create(
			context, "loop.cond", builder.GetInsertBlock()->getParent());
	const auto& loopBody = BasicBlock::Create(
			context, "loop.body", builder.GetInsertBlock()->getParent());
	const auto& loopEnd = BasicBlock::Create(
			context, "loop.end", builder.GetInsertBlock()->getParent());
	const auto& loopIter = loopStatement.iteration ? BasicBlock::Create(context, "loop.iter",
			builder.GetInsertBlock()->getParent()) : nullptr;

	for (const auto& init: loopStatement.init) {
		init->visit(*this);
	}
	builder.CreateBr(loopStatement.doWhile ? loopBody : loopCond);

	builder.SetInsertPoint(loopCond);
	if (loopStatement.condition) {
		ret = LRValue(loopStatement.condition, true);
		ret = cast(ret, builder.getInt1Ty());
		builder.CreateCondBr(ret, loopBody, loopEnd);
	}
	else
		builder.CreateBr(loopBody);

	builder.SetInsertPoint(loopBody);
	const auto breakBackup = breakBlock;
	const auto continueBackup = continueBlock;
	breakBlock = loopEnd;
	continueBlock = loopIter ? loopIter : loopCond;
	loopStatement.body->visit(*this);
	breakBlock = breakBackup;
	continueBlock = continueBackup;
	builder.CreateBr(loopIter ? loopIter : loopCond);

	if (loopIter) {
		builder.SetInsertPoint(loopIter);
		loopStatement.iteration->visit(*this);
		builder.CreateBr(loopCond);
	}

	builder.SetInsertPoint(loopEnd);
}

void IRVisitor::visitControlStatement(
		const Ast::ControlStatement& controlStatement)
{
	if (controlStatement.name()=="break")
		builder.CreateBr(breakBlock);
	else
		builder.CreateBr(continueBlock);
}

void IRVisitor::visitReturnStatement(
		const Ast::ReturnStatement& returnStatement)
{
	if (returnStatement.expr) {
		ret = LRValue(returnStatement.expr, true);
		ret = cast(ret, builder.getCurrentFunctionReturnType());
		builder.CreateRet(ret);
	}
	else builder.CreateRetVoid();
}

void IRVisitor::visitFunctionDefinition(
		const Ast::FunctionDefinition& functionDefinition)
{
	const auto& function = getOrCreateFunction(
			functionDefinition.identifier, functionDefinition.table);
	const auto& returnType = function->getReturnType();
	const auto& block = BasicBlock::Create(context, "entry", function);
	builder.SetInsertPoint(block);
	size_t i = 0;
	for (auto& parameter : function->args()) {
		const auto& name = functionDefinition.parameters[i++].second;
		ret = createAlloca(parameter.getType(), name);
		functionDefinition.body->table->lookup(name)->allocaInst = ret;
		builder.CreateStore(&parameter, ret);
	}
	functionDefinition.body->visit(*this);
	if (returnType->isVoidTy())
		builder.CreateRetVoid();
	else
		builder.CreateRet(functionDefinition.identifier=="main"
		                  ? Constant::getNullValue(returnType)
		                  : UndefValue::get(returnType));
	RemoveUnusedCodeInBlockPass removeUnusedCode;
	removeUnusedCode.runOnFunction(*function);
}

void IRVisitor::visitFunctionCall(const Ast::FunctionCall& functionCall)
{
	const auto& function = llvm::cast<Function>(
			*functionCall.table->lookupAllocaInst(functionCall.value()));
	std::vector<Value*> arguments;
	for (int i = 0; i<functionCall.arguments.size(); ++i) {
		const auto& argument = functionCall.arguments[i];
		ret = LRValue(argument, true);
		if (!function->isVarArg() || i<function->arg_size()) ret = cast(ret, function->args().begin()[i].getType());
		else if (ret->getType()->isFloatTy()) ret = cast(ret, builder.getDoubleTy());
		arguments.emplace_back(ret);
	}
	ret = builder.CreateCall(function, arguments);
}

void IRVisitor::visitSubscriptExpr(const Ast::SubscriptExpr& subscriptExpr)
{
	const auto rhs = LRValue(subscriptExpr.rhs, true);
	ret = LRValue(subscriptExpr.lhs, true, rhs);
	isRvalue = false;
}

void IRVisitor::visitIncludeStdioStatement(
		const Ast::IncludeStdioStatement& includeStdioStatement)
{
	auto names = {"printf", "scanf"};
	for (const auto& name : names) {
		const auto& printf = module.getOrInsertFunction(name,
				llvm::FunctionType::get(llvm::Type::getInt32PtrTy(context), builder.getInt8PtrTy(), true)).getCallee();
		includeStdioStatement.table->lookup(name)->allocaInst = printf;
	}
}

void IRVisitor::visitFunctionDeclaration(
		const Ast::FunctionDeclaration& functionDeclaration)
{
	getOrCreateFunction(functionDeclaration.identifier, functionDeclaration.table);
}

llvm::Value* IRVisitor::cast(llvm::Value* value, llvm::Type* to)
{
	auto from = value->getType();
	if (from==to)
		return value;

	if (from==builder.getInt1Ty()) {
		if (to->isIntegerTy())
			return builder.CreateZExt(value, to);
		else if (to->isFloatTy())
			return builder.CreateFPToUI(value, to);
		else if (to->isPointerTy())
			return builder.CreateIntToPtr(value, to);
	}
	else if (from->isIntegerTy()) {
		if (to==builder.getInt1Ty())
			return builder.CreateICmpNE(value, Constant::getNullValue(from));
		else if (to->isIntegerTy())
			return builder.CreateSExtOrTrunc(value, to);
		else if (to->isFloatTy())
			return builder.CreateSIToFP(value, to);
		else if (to->isPointerTy())
			return builder.CreateIntToPtr(value, to);
	}
	else if (from->isFloatTy()) {
		if (to==builder.getInt1Ty())
			return builder.CreateFCmpUNE(value, Constant::getNullValue(from));
		else if (to->isIntegerTy())
			return builder.CreateFPToSI(value, to);
		else if (to->isDoubleTy())
			return builder.CreateFPExt(value, to);
	}
	else if (from->isPointerTy()) {
		if (to==builder.getInt1Ty())
			return builder.CreateICmpNE(value, Constant::getNullValue(from));
		else if (to->isIntegerTy())
			return builder.CreatePtrToInt(value, to);
		else if (to->isPointerTy())
			return builder.CreatePointerCast(value, to);
	}
	throw InternalError("Invalid cast expression in LLVM IR");
}

llvm::Value* IRVisitor::increaseOrDecrease(const bool inc, llvm::Value* input)
{
	auto type = input->getType();
	std::string opName = inc ? "inc" : "dec";
	if (type->isFloatTy())
		return builder.CreateBinOp(inc ? Instruction::FAdd : Instruction::FSub,
				input, ConstantFP::get(builder.getFloatTy(), 1),
				opName);
	else if (type->isPointerTy())
		return builder.CreateInBoundsGEP(input, builder.getInt32(inc*2-1),
				opName);
	else
		return builder.CreateBinOp(inc ? Instruction::Add : Instruction::Sub, input,
				ConstantInt::get(type, 1), opName);
}

llvm::Type* IRVisitor::convertToIR(::Type* type, const bool function,
		const bool first)
{
	if (first)
		type = ::Type::invert(type);
	if (type->isBaseType()) {
		switch (type->getBaseType()) {
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
	else if (type->isPointerType()) {
		return PointerType::getUnqual(
				convertToIR(type->getDerefType(), false, false));
	}
	else if (type->isVoidType()) {
		if (function)
			return builder.getVoidTy();
		return builder.getInt8Ty();
	}
	else if (type->isArrayType()) {
		return llvm::ArrayType::get(convertToIR(type->getDerefType(), false, false),
				type->getArrayType().first);
	}
	throw IRError(type->string());
}

AllocaInst* IRVisitor::createAlloca(llvm::Type* type, const std::string& name)
{
	auto& block = builder.GetInsertBlock()->getParent()->getEntryBlock();
	IRBuilder tmpBuilder(&block, block.begin());
	return tmpBuilder.CreateAlloca(type, nullptr, name);
}

Value* IRVisitor::LRValue(Ast::Node* ASTValue, const bool requiresRvalue,
		Value* inc)
{
	ASTValue->visit(*this);
	auto value = ret;
	const auto& type = value->getType();

	//	errs() << "in: " << *type << "\t";
	//	errs().flush();

	if (requiresRvalue && !isRvalue) {
		const auto& containedType = type->getContainedType(0);
		if (containedType->isArrayTy()) {
			value = builder.CreateInBoundsGEP(
					value, {builder.getInt64(0), inc ? inc : builder.getInt64(0)});
		}
		else {
			value = builder.CreateLoad(value);
			if (inc)
				value = builder.CreateInBoundsGEP(value, inc);
		}
	}
	else if (inc) {
		value = builder.CreateInBoundsGEP(value, inc);
	}

	//	errs() << "out: " << *value->getType() << '\n';
	//	errs().flush();

	isRvalue = true;
	return value;
}

llvm::Function* IRVisitor::getOrCreateFunction(const std::string& identifier, const std::shared_ptr<SymbolTable> table)
{
	const auto& ASTFunction = table->lookup(identifier);
	if (ASTFunction->allocaInst)
		return llvm::cast<Function>(ASTFunction->allocaInst);

	std::vector<llvm::Type*> llvmParameters;
	const auto& type = ASTFunction->type->getFunctionType();
	for (const auto& parameter : type.parameters) {
		llvmParameters.emplace_back(convertToIR(parameter));
	}
	const auto& llvmReturnType = convertToIR(type.returnType, true);
	const auto& functionType =
			llvm::FunctionType::get(llvmReturnType, llvmParameters, false);
	const auto function = llvm::cast<Function>(
			module.getOrInsertFunction(identifier, functionType).getCallee());
	table->lookup(identifier)->allocaInst = function;
	return function;
}

llvm::Module& IRVisitor::getModule()
{
	return module;
}
