//============================================================================
// @author      : Thomas Dooms & Ward Gauderis
// @date        : 3/10/20
// @copyright   : BA2 Informatica - Thomas Dooms & Ward Gauderis - University of Antwerp
//============================================================================
#pragma once

#include <array>
#include <memory>
#include <optional>
#include <utility>
#include <variant>
#include <vector>
#include "errors.h"

namespace Ast {

	struct Node {
		explicit Node() = default;

		friend std::ofstream& operator<<(std::ofstream& stream, const std::unique_ptr<Node>& root);

		[[nodiscard]] virtual std::string name() const = 0;

		[[nodiscard]] virtual std::string value() const = 0;

		[[nodiscard]] virtual std::vector<Node*> children() const = 0;

		[[nodiscard]] virtual std::string color() const = 0;
	};

	struct Statement : public Node {
		Statement() = default;

		[[nodiscard]] std::string color() const override;
	};

	struct Expr : public Statement {
		explicit Expr() = default;

		[[nodiscard]] std::string color() const final;
	};

	struct Comment final : public Node {
		explicit Comment(std::string comment)
				:comment(std::move(comment)) { }

		[[nodiscard]] std::string name() const final;

		[[nodiscard]] std::string value() const final;

		[[nodiscard]] std::vector<Node*> children() const final;

		[[nodiscard]] std::string color() const final;

		std::string comment;
	};

	struct Block final : public Node {
		explicit Block(std::vector<Node*> nodes)
				:nodes(std::move(nodes)) { }

		std::vector<Node*> nodes;

		[[nodiscard]] std::string name() const final;

		[[nodiscard]] std::string value() const final;

		[[nodiscard]] std::vector<Node*> children() const final;

		[[nodiscard]] std::string color() const final;
	};

	struct Literal final : public Expr {
		template<typename Type>
		explicit Literal(Type val)
				: literal(val) { }

		[[nodiscard]] std::string name() const final;

		[[nodiscard]] std::string value() const final;

		[[nodiscard]] std::vector<Node*> children() const final;

		std::variant<char, short, int, long, float, double> literal;
	};

	struct Type : public Node {
		bool isConst;

		explicit Type(bool isConst)
				:isConst(isConst) { }

		[[nodiscard]] std::string name() const final;

		[[nodiscard]] std::string color() const final;
	};

	struct BasicType : public Type {
		std::string type;

		BasicType(bool isConst, std::string type)
				:Type(isConst), type(std::move(type)) { }

		[[nodiscard]] std::string value() const final;

		[[nodiscard]] std::vector<Node*> children() const final;
	};

	struct PointerType : public Type {
		Type* baseType;

		[[nodiscard]] std::string value() const final;

		PointerType(bool isConst, Type* baseType)
				:Type(isConst), baseType(baseType) { }

		[[nodiscard]] std::vector<Node*> children() const final;
	};

	//TODO: give variable a value?
	struct Variable final : public Expr {
		explicit Variable(Type* type, std::string identifier)
				:type(type), identifier(std::move(identifier)) { }

		[[nodiscard]] std::string name() const final;

		[[nodiscard]] std::string value() const final;

		[[nodiscard]] std::vector<Node*> children() const final;

		Type* type;
		std::string identifier;
	};

	struct BinaryExpr final : public Expr {
		explicit BinaryExpr(std::string operation, Expr* lhs, Expr* rhs)
				:operation(std::move(operation)), lhs(lhs), rhs(rhs) { }

		[[nodiscard]] std::string name() const override;

		[[nodiscard]] std::string value() const override;

		[[nodiscard]] std::vector<Node*> children() const override;

		std::string operation;

		Expr* lhs;
		Expr* rhs;
	};

	struct PostfixExpr final : public Expr {
		explicit PostfixExpr(std::string operation, Variable* variable)
				:operation(std::move(operation)), variable(variable) { }

		[[nodiscard]] std::string name() const final;

		[[nodiscard]] std::string value() const final;

		[[nodiscard]] std::vector<Node*> children() const final;

		std::string operation;
		Variable* variable;
	};

	struct PrefixExpr final : public Expr {
		explicit PrefixExpr(std::string operation, Variable* variable)
				:operation(std::move(operation)), variable(variable) { }

		[[nodiscard]] std::string name() const final;

		[[nodiscard]] std::string value() const final;

		[[nodiscard]] std::vector<Node*> children() const final;

		std::string operation;
		Variable* variable;
	};

	struct UnaryExpr final : public Expr {
		explicit UnaryExpr(std::string operation, Expr* operand)
				:operation(std::move(operation)), operand(operand) { }

		[[nodiscard]] std::string name() const final;

		[[nodiscard]] std::string value() const final;

		[[nodiscard]] std::vector<Node*> children() const final;

		std::string operation;
		Expr* operand;
	};

	struct CastExpr final : public Expr {
		explicit CastExpr(Type* type, Expr* operand)
				:type(type), operand(operand) { }

		[[nodiscard]] std::string name() const final;

		[[nodiscard]] std::string value() const final;

		[[nodiscard]] std::vector<Node*> children() const final;

		Type* type;
		Expr* operand;
	};

	struct Assignment final : public Expr {
		explicit Assignment(Variable* variable, Expr* expr)
				:variable(variable), expr(expr) { }

		[[nodiscard]] std::string name() const final;

		[[nodiscard]] std::string value() const final;

		[[nodiscard]] std::vector<Node*> children() const final;

		Variable* variable;
		Expr* expr;
	};

	struct Declaration final : public Statement {
		explicit Declaration(Variable* variable, Expr* expr)
				:variable(variable), expr(expr) { }

		[[nodiscard]] std::string name() const final;

		[[nodiscard]] std::string value() const final;

		[[nodiscard]] std::vector<Node*> children() const final;

		Variable* variable;

		Expr* expr;  // can be nullptr
	};

	struct PrintfStatement final : public Statement {
		explicit PrintfStatement(Expr* expr)
				:expr(expr) { }

		[[nodiscard]] std::string name() const final;

		[[nodiscard]] std::string value() const final;

		[[nodiscard]] std::vector<Node*> children() const final;

		Expr* expr;
	};


}

// helpers for the ast traversal
template<class... Ts>
struct overloaded : Ts ... {
	using Ts::operator()...;
};
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

template<typename Func>
void downcast_expr(Ast::Expr* node, const Func& func)
{
	if (auto* res = dynamic_cast<Ast::BinaryExpr*   >(node)) func(res);
	else if (auto* res = dynamic_cast<Ast::UnaryExpr*    >(node)) func(res);
	else if (auto* res = dynamic_cast<Ast::Literal*      >(node)) func(res);
	else if (auto* res = dynamic_cast<Ast::Variable*     >(node)) func(res);
	else if (auto* res = dynamic_cast<Ast::Assignment*   >(node)) func(res);
	else throw SyntaxError("unknown expression type");
}

template<typename Func>
void downcast_statement(Ast::Statement* node, const Func& func)
{
	if (auto* res = dynamic_cast<Ast::Declaration*    >(node)) func(res);
	else if (auto* res = dynamic_cast<Ast::Expr*  >(node)) func(res);
	else if (auto* res = dynamic_cast<Ast::PrintfStatement*>(node)) func(res);
	else throw SyntaxError("unknown statement type");
}

template<typename Func>
void downcast_node(Ast::Node* node, const Func& func)
{
	if (auto* res = dynamic_cast<Ast::Comment*      >(node)) func(res);
	else if (auto* res = dynamic_cast<Ast::Block*        >(node)) func(res);
	else if (auto* res = dynamic_cast<Ast::Expr*         >(node)) downcast_expr(res, func);
	else if (auto* res = dynamic_cast<Ast::Statement*    >(node)) downcast_statement(res, func);
	else throw SemanticError("unknown node type");
}






