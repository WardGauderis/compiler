//============================================================================
// @name        : folding.h
// @author      : Thomas Dooms
// @date        : 3/10/20
// @version     :
// @copyright   : BA1 Informatica - Thomas Dooms - University of Antwerp
// @description :
//============================================================================

#pragma once

#include "ast.h"

Ast::Expr* foldExpr(Ast::Expr* expr);

std::unique_ptr<Ast::Block> foldNodes(const std::vector<Ast::Node*>& nodes);