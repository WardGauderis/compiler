//
// Created by ward on 3/17/20.
//

#include "ast.h"
#include "errors.h"
#include <filesystem>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>

using namespace llvm;

namespace Ast
{
static LLVMContext context;
static Module module("temp", context);
static IRBuilder builder(context);

void ast2ir(const std::unique_ptr<Ast::Node>& root, std::filesystem::path& path)
{
    module.getOrInsertFunction(
        "printf", llvm::FunctionType::get(llvm::Type::getInt32PtrTy(context), builder.getInt8PtrTy(), true));

    FunctionCallee tmp = module.getOrInsertFunction("main", builder.getInt32Ty());
    auto main          = cast<Function>(tmp.getCallee());

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
    return nullptr;
}

llvm::Value* Block::codegen() const
{
    for (const auto& node : nodes)
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

// TODO alloca
llvm::Value* Variable::codegen() const
{
    return nullptr;
}

llvm::Value* BinaryExpr::codegen() const
{
    auto l              = lhs->codegen();
    auto r              = rhs->codegen();
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

    if (operation.type == BinaryOperation::Mul)
        return builder.CreateBinOp(floatOperation ? Instruction::FMul : Instruction::Mul, l, r);
    if (operation.type == BinaryOperation::Div)
        return builder.CreateBinOp(floatOperation ? Instruction::FDiv : Instruction::SDiv, l, r);
    if (operation.type == BinaryOperation::Mod) return builder.CreateBinOp(Instruction::SRem, l, r);
    if (operation.type == BinaryOperation::Add)
        return builder.CreateBinOp(floatOperation ? Instruction::FAdd : Instruction::Add, l, r);
    if (operation.type == BinaryOperation::Sub)
        return builder.CreateBinOp(floatOperation ? Instruction::FSub : Instruction::Sub, l, r);
    if (operation.type == BinaryOperation::Lt)
        return floatOperation ? builder.CreateFCmp(CmpInst::FCMP_OLT, l, r)
                              : builder.CreateICmp(CmpInst::ICMP_SLT, l, r);
    if (operation.type == BinaryOperation::Le)
        return floatOperation ? builder.CreateFCmp(CmpInst::FCMP_OLE, l, r)
                              : builder.CreateICmp(CmpInst::ICMP_SLE, l, r);
    if (operation.type == BinaryOperation::Gt)
        return floatOperation ? builder.CreateFCmp(CmpInst::FCMP_OGT, l, r)
                              : builder.CreateICmp(CmpInst::ICMP_SGT, l, r);
    if (operation.type == BinaryOperation::Ge)
        return floatOperation ? builder.CreateFCmp(CmpInst::FCMP_OGE, l, r)
                              : builder.CreateICmp(CmpInst::ICMP_SGE, l, r);
    if (operation.type == BinaryOperation::Eq)
        return floatOperation ? builder.CreateFCmp(CmpInst::FCMP_OEQ, l, r)
                              : builder.CreateICmp(CmpInst::ICMP_EQ, l, r);
    if (operation.type == BinaryOperation::Neq)
        return floatOperation ? builder.CreateFCmp(CmpInst::FCMP_UNE, l, r)
                              : builder.CreateICmp(CmpInst::ICMP_NE, l, r);
    if (operation.type == BinaryOperation::And) // TODO logical
        return builder.CreateBinOp(floatOperation ? Instruction::And : Instruction::Mul, l, r);
    if (operation.type == BinaryOperation::Or)
        return builder.CreateBinOp(floatOperation ? Instruction::Or : Instruction::Mul, l, r);
    // TODO or and float pointer
    throw InternalError("type is not supported in IR");
}

llvm::Value* PostfixExpr::codegen() const
{
    return nullptr;
}

llvm::Value* PrefixExpr::codegen() const
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
    auto result = expr->codegen();
    std::string format;
    std::string name;
    if (expr->type().isPointerType())
    {
        format = "%p\n";
        name   = "ptrFormat";
    }
    else if (expr->type().isFloatingType())
    {
        format = "%f\n";
        name   = "floatFormat";
    }
    else if (expr->type().isCharacterType())
    {
        format = "%c\n";
        name   = "charFormat";
    }
    else if (expr->type().isIntegerType())
    {
        format = "%d\n";
        name   = "intFormat";
    }
    else
        throw InternalError("ype is not supported in IR");
    return builder.CreateCall(
        module.getFunction("printf"), {builder.CreateGlobalStringPtr(format, name), result});
}
} // namespace Ast