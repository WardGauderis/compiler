//
// Created by ward on 3/6/20.
//

#include <iostream>
#include "ast.h"

void printNode(std::ostream& os, const AstNode* node) {
	os << '"' << node << '"' << " [label=\"" << node->getSymbol() << "\"];\n";
}

void print(std::ostream& os, const AstNode* node, const AstNode* parent) {
	printNode(os, node);
	os << '"' << parent << '"' << " -> " << '"' << node << '"' << ";\n";
}

std::ostream& operator<<(std::ostream& os, AstNode& node) {
	os << "digraph G {\n";
	os << "0 [style = invis];\n";
	print(os, &node, nullptr);
	node.serialize(os);
	os << "}\n" << std::flush;
	return os;
}

std::string File::getSymbol() const {
	return "file";
}

void File::serialize(std::ostream& os) const {
	for (const auto& expr : exprs) {
		print(os, expr, this);
		expr->serialize(os);
	}
}

void BinaryExpr::serialize(std::ostream& os) const {
	for (const auto& operand: operands) {
		print(os, operand, this);
		operand->serialize(os);
	}
}

std::string BinaryExpr::getSymbol() const {
	switch (type) {
		case MUL:
			return "*";
		case DIV:
			return "/";
		case MOD:
			return "%";
		case PLU:
			return "+";
		case MIN:
			return "-";
		case LES:
			return "<";
		case LOE:
			return "<=";
		case MOR:
			return ">";
		case MOE:
			return ">=";
		case EQU:
			return "==";
		case NEQ:
			return "!=";
		case AND:
			return "&&";
		case OR:
			return "||";
	}
}

void UnaryExpr::serialize(std::ostream& os) const {
	print(os, operand, this);
	operand->serialize(os);
}

std::string UnaryExpr::getSymbol() const {
	switch (type) {
		case PLU:
			return "+";
		case MIN:
			return "-";
		case NOT:
			return "!";
	}
}

void Int::serialize(std::ostream& os) const {}

std::string Int::getSymbol() const {
	return std::to_string(value);
}
