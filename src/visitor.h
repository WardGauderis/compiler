//============================================================================
// @author      : Thomas Dooms & Ward Gauderis
// @date        : 3/10/20
// @copyright   : BA2 Informatica - Thomas Dooms & Ward Gauderis - University of Antwerp
//============================================================================

#pragma once

#include <memory>
#include <tree/ParseTree.h>

#include "ast/expressions.h"
#include "ast/node.h"
#include "ast/statements.h"

#include "cst.h"
#include "errors.h"

Ast::Comment* visitComment(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table);

Ast::Literal* visitLiteral(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table);

Ast::Expr* visitLiteralOrVariable(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table);

Ast::Expr* visitBasicExpr(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table);

Ast::Expr* visitPostfixExpr(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table);

Ast::Expr* visitPrefixExpr(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table);

Ast::Expr* visitMultiplicativeExpr(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table);

Ast::Expr* visitAdditiveExpr(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table);

Ast::Expr* visitRelationalExpr(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table);

Ast::Expr* visitEqualityExpr(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table);

Ast::Expr* visitAndExpr(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table);

Ast::Expr* visitOrExpr(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table);

Ast::Expr* visitAssignExpr(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table);

Type visitTypeName(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table);

Type visitBasicType(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table);

Type visitPointerType(antlr4::tree::ParseTree* context, Type type, std::shared_ptr<SymbolTable>& table);

Ast::Expr* visitInitializer(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table);

Ast::Statement* visitDeclaration(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table);

Ast::Expr* visitExpr(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table);

//Ast::Expr* visitPrintf(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table);

Ast::Scope* visitScopeStatement(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& parent, ScopeType type);

Ast::Statement* visitIfStatement(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table);

Ast::Statement* visitWhileStatement(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table);

Ast::Statement* visitForStatement(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table);

Ast::Expr* visitExprStatement(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table);

Ast::Statement* visitControlStatement(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table);

Ast::Statement* visitStatement(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table, ScopeType type);

std::vector<std::pair<Type, std::string>> visitParameterList(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table);

std::vector<Ast::Expr*> visitArgumentList(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table);

Ast::Statement* visitFunctionDefinition(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& parent);

Ast::Scope* visitFile(antlr4::tree::ParseTree* context);

namespace Ast
{
std::unique_ptr<Ast::Node> from_cst(const std::unique_ptr<Cst::Root>& root, bool fold);
}