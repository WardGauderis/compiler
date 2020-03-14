//============================================================================
// @author      : Thomas Dooms & Ward Gauderis
// @date        : 3/10/20
// @copyright   : BA2 Informatica - Thomas Dooms & Ward Gauderis - University of Antwerp
//============================================================================

#pragma once

#include <memory>
#include <tree/ParseTree.h>

#include "CParser.h"
#include "ast.h"
#include "errors.h"

Ast::Comment* visitComment(antlr4::tree::ParseTree* context);

Ast::Literal* visitLiteral(antlr4::tree::ParseTree* context);

Ast::Expr* visitLiteralOrVariable(antlr4::tree::ParseTree* context);

Ast::Expr* visitBasicExpr(antlr4::tree::ParseTree* context);

Ast::Expr* visitPostfixExpr(antlr4::tree::ParseTree* context);

Ast::Expr* visitprefixExpr(antlr4::tree::ParseTree* context);

Ast::Expr* visitUnaryExpr(antlr4::tree::ParseTree* context);

Ast::Expr* visitMultiplicativeExpr(antlr4::tree::ParseTree* context);

Ast::Expr* visitAdditiveExpr(antlr4::tree::ParseTree* context);

Ast::Expr* visitRelationalyExpr(antlr4::tree::ParseTree* context);

Ast::Expr* visitEqualityExpr(antlr4::tree::ParseTree* context);

Ast::Expr* visitAndExpr(antlr4::tree::ParseTree* context);

Ast::Expr* visitOrExpr(antlr4::tree::ParseTree* context);

Ast::Expr* visitAssignExpr(antlr4::tree::ParseTree* context);

Ast::Expr* visitExpr(antlr4::tree::ParseTree* context);

Ast::Type* visitTypeName(antlr4::tree::ParseTree* context);

Ast::BasicType* visitBasicType(antlr4::tree::ParseTree* context);

Ast::PointerType* visitPointerType(antlr4::tree::ParseTree* context, Ast::Type* baseType);

Ast::Expr* visitInitializer(antlr4::tree::ParseTree* context);

Ast::Statement* visitDeclaration(antlr4::tree::ParseTree* context);

Ast::Statement* visitPrintf(antlr4::tree::ParseTree* context);

Ast::Statement* visitStatement(antlr4::tree::ParseTree* context);

std::unique_ptr<Ast::Node> visitBlock(antlr4::tree::ParseTree* context);