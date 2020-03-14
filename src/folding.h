//============================================================================
// @author      : Thomas Dooms & Ward Gauderis
// @date        : 3/10/20
// @copyright   : BA2 Informatica - Thomas Dooms & Ward Gauderis - University of Antwerp
//============================================================================

#pragma once

#include "ast.h"

Ast::Expr* foldExpr(Ast::Expr* expr);

std::unique_ptr<Ast::Block> foldNodes(const std::vector<Ast::Node*>& nodes);