//
// Created by ward on 3/6/20.
//

#ifndef COMPILER_AST_H
#define COMPILER_AST_H

#include <vector>
#include <array>

class AstNode {
public:
	friend std::ostream& operator<<(std::ostream& os, AstNode& node);

	virtual void serialize(std::ostream& os) const = 0;

private:
	friend void printNode(std::ostream& os, const AstNode* node);

	friend void print(std::ostream& os, const AstNode* node, const AstNode* parent);

	[[nodiscard]] virtual std::string getSymbol() const = 0;
};

class Expr : public AstNode {

};

class File : public AstNode {
public:
	std::vector<Expr*> exprs;

private:
	[[nodiscard]] std::string getSymbol() const final;

	void serialize(std::ostream& os) const final;
};

class BinaryExpr : public Expr {
public:
	enum Type {
		MUL, DIV, MOD, PLU, MIN, LES, LOE, MOR, MOE, EQU, NEQ, AND, OR,
	};

	Type type;

	std::array<Expr*, 2> operands;

private:
	void serialize(std::ostream& os) const final;

	[[nodiscard]] std::string getSymbol() const final;
};

class UnaryExpr : public Expr {
public:
	enum Type {
		PLU, MIN, NOT
	};

	Type type;

	Expr* operand;

private:
	void serialize(std::ostream& os) const final;

	[[nodiscard]] std::string getSymbol() const final;
};

class Literal : public Expr {

};

class Int : public Literal {
public:
	int value;

private:
	void serialize(std::ostream& os) const override;

	[[nodiscard]] std::string getSymbol() const override;
};

#endif //COMPILER_AST_H
