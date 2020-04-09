//============================================================================
// @author      : Thomas Dooms
// @date        : 3/21/20
// @copyright   : BA2 Informatica - Thomas Dooms - University of Antwerp
//============================================================================

#include "expressions.h"
#include "helper.h"
#include <IRVisitor/irVisitor.h>

namespace Ast {
	std::string Expr::color() const
	{
		return "#ced6eb"; // light blue
	}

	std::string Comment::name() const
	{
		return "comment";
	}

	std::string Comment::value() const
	{
		return "...";
	}

	std::string Comment::color() const
	{
		return "#d5ceeb"; // light purple
	}

	void Comment::visit(IRVisitor& visitor)
	{
		visitor.visitComment(*this);
	}

	std::string Literal::name() const
	{
		return "literal";
	}

	std::string Literal::value() const
	{
		return std::visit([&](const auto& val)
		{ return std::to_string(val); }, literal);
	}

	Literal* Literal::fold()
	{
		return this;
	}

	Type* Literal::type() const
	{
		return new Type(true, static_cast<BaseType>(literal.index()));
	}

	bool Literal::constant() const
	{
		return true;
	}

	void Literal::visit(IRVisitor& visitor)
	{
		visitor.visitLiteral(*this);
	}

	std::string StringLiteral::name() const
	{
		return "literal";
	}

	std::string StringLiteral::value() const
	{
		return val;
	}

	Node* StringLiteral::fold()
	{
		return this;
	}

	Type* StringLiteral::type() const
	{
		// +1 is for the null terminator
		return new Type(true, val.size()+1, new Type(true, BaseType::Char));
	}

	bool StringLiteral::constant() const
	{
		return true;
	}

	void StringLiteral::visit(IRVisitor& visitor)
	{
		visitor.visitStringLiteral(*this);
	}

	std::string Variable::name() const
	{
		return identifier;
	}

	std::string Variable::value() const
	{
		return table->lookup(identifier)->type->string();
	}

	std::string Variable::color() const
	{
		return "#ebe6ce";
	}

	Node* Variable::fold()
	{
		if (auto* res = table->lookup(identifier))
		{
			if (not res->literal.has_value())
			{
                return this;
			}
			else
            {
                return new Ast::Literal(res->literal.value(), table, line, column);
            }
		}
		throw InternalError("variable not found while folding");
	}

	bool Variable::check() const
	{
		if (auto* res = table->lookup(identifier))
		{
			if (not res->isInitialized)
			{
				std::cout << UninitializedWarning(identifier, line, column);
				res->isInitialized = true;
			}
		}
		else
		{
			std::cout << UndeclaredError(identifier, line, column);
			return false;
		}
		return true;
	}

	Type* Variable::type() const
	{
		if (auto* res = table->lookup(identifier)) return res->type;
		else
			return new Type;
	}

	bool Variable::constant() const
	{
		return false;
	}

	void Variable::visit(IRVisitor& visitor)
	{
		visitor.visitVariable(*this);
	}

	std::string BinaryExpr::name() const
	{
		return "binary expression";
	}

	std::string BinaryExpr::value() const
	{
		return operation.string();
	}

	std::vector<Node*> BinaryExpr::children() const
	{
		return {lhs, rhs};
	}

	Node* BinaryExpr::fold()
	{
		Helper::folder(lhs);
		Helper::folder(rhs);

		const auto res0 = dynamic_cast<Literal*>(lhs);
		const auto res1 = dynamic_cast<Literal*>(rhs);

		if (res0 and res1)
		{
			const auto lambda = [&](const auto& val0, const auto& val1)
			{
				return Helper::fold_binary(val0, val1, operation, table, line, column);
			};
			return std::visit(lambda, res0->literal, res1->literal);
		}
		return this;
	}

	bool BinaryExpr::check() const
	{
		return Type::combine(operation, lhs->type(), rhs->type(), line, column)!=nullptr;
	}

	Type* BinaryExpr::type() const
	{
		try
		{
			return Type::combine(operation, lhs->type(), rhs->type(), 0, 0, false);
		}
		catch (...)
		{
			return new Type;
		}
	}

	bool BinaryExpr::constant() const
	{
		return rhs->constant() && lhs->constant();
	}

	void BinaryExpr::visit(IRVisitor& visitor)
	{
		visitor.visitBinaryExpr(*this);
	}

	std::string PrefixExpr::name() const
	{
		return "prefix expression";
	}

	std::string PrefixExpr::value() const
	{
		return operation.string()+operand->value();
	}

	std::vector<Node*> PrefixExpr::children() const
	{
		return {operand};
	}

	Node* PrefixExpr::fold()
	{
		// I cannot constant fold variables when taking addrof
		if (operation!=PrefixOperation::Addr)
		{
			Helper::folder(operand);
		}

		if (auto* res = dynamic_cast<Ast::Literal*>(operand))
		{
			const auto lambda
					= [&](const auto& val)
					{ return Helper::fold_prefix(val, operation, table, line, column); };
			return std::visit(lambda, res->literal);
		}
		return this;
	}

	bool PrefixExpr::check() const
	{
		if (operation.isIncrDecr() and operand->type()->isConst())
		{
			std::cout << ConstError("prefix operator", operand->name(), line, column);
			return false;
		}

		if (not Helper::is_lvalue(operand))
		{
			if (operation.isIncrDecr())
			{
				std::cout << RValueError("assigning to", line, column);
				return false;
			}
		}

		return Type::unary(operation, operand->type(), line, column)!=nullptr;
	}

	Type* PrefixExpr::type() const
	{
		try
		{
			return Type::unary(operation, operand->type(), 0, 0, false);
		}
		catch (...)
		{
			return new Type;
		}
	}

	bool PrefixExpr::constant() const
	{
		return operand->constant();
	}

	void PrefixExpr::visit(IRVisitor& visitor)
	{
		visitor.visitPrefixExpr(*this);
	}

	std::string PostfixExpr::name() const
	{
		return "prefix expression";
	}

	std::string PostfixExpr::value() const
	{
		return operation.string()+operand->value();
	}

	std::vector<Node*> PostfixExpr::children() const
	{
		return {operand};
	}

	Node* PostfixExpr::fold()
	{
		Helper::folder(operand);

		auto* res = dynamic_cast<Ast::Literal*>(operand);
		const auto lambda
				= [&](const auto& val)
				{ return Helper::fold_postfix(val, operation, table, line, column); };

		// TODO: deletus feetus, memory leakus
		if (res) return std::visit(lambda, res->literal);
		return this;
	}

	bool PostfixExpr::check() const
	{
		if (not Helper::is_lvalue(operand))
		{
			std::cout << RValueError("assigning to", line, column);
			return false;
		}

		if (operand->type()->isConst())
		{
			std::cout << ConstError("postfix expr", operand->name(), line, column);
			return false;
		}
		return true;
	}

	Type* PostfixExpr::type() const
	{
		return operand->type();
	}

	bool PostfixExpr::constant() const
	{
		return false;
	}

	void PostfixExpr::visit(IRVisitor& visitor)
	{
		visitor.visitPostfixExpr(*this);
	}

	std::string CastExpr::name() const
	{
		return "cast expression";
	}

	std::string CastExpr::value() const
	{
		return '('+cast->string()+')';
	}

	std::vector<Node*> CastExpr::children() const
	{
		return {operand};
	}

	Node* CastExpr::fold()
	{
		Helper::folder(operand);

		auto* res = dynamic_cast<Ast::Literal*>(operand);
		const auto lambda
				= [&](const auto& val)
				{ return Helper::fold_cast(val, cast, table, line, column); };

		if (res) return std::visit(lambda, res->literal);
		else
			return this;
	}

	bool CastExpr::check() const
	{
		return Type::convert(operand->type(), cast, true, line, column);
	}

	Type* CastExpr::type() const
	{
		return cast;
	}

	bool CastExpr::constant() const
	{
		return operand->constant();
	}

	void CastExpr::visit(IRVisitor& visitor)
	{
		visitor.visitCastExpr(*this);
	}

	std::string Assignment::name() const
	{
		return "assignment";
	}

	std::vector<Node*> Assignment::children() const
	{
		return {lhs, rhs};
	}

	Node* Assignment::fold()
	{
		Helper::folder(rhs);
		return this;
	}

	bool Assignment::check() const
	{
		if (not Helper::is_lvalue(lhs))
		{
			std::cout << RValueError("assigning to", line, column);
			return false;
		}

		if(auto* res = dynamic_cast<Variable*>(lhs))
		{
                  table->lookup(res->identifier)->isInitialized = true;
		}

		if(lhs->type()->isConst())
		{
                  std::cout << ConstError("assigning to", lhs->name(), line, column);
		  return false;
		}

		return Type::convert(rhs->type(), lhs->type(), false, line, column);
	}

	Type* Assignment::type() const
	{
		return lhs->type();
	}

	bool Assignment::constant() const
	{
		return false;
	}

	void Assignment::visit(IRVisitor& visitor)
	{
		visitor.visitAssignment(*this);
	}

	std::string FunctionCall::name() const
	{
		return "function call";
	}

	std::string FunctionCall::value() const
	{
		return identifier;
	}

	std::vector<Node*> FunctionCall::children() const
	{
		return std::vector<Node*>(arguments.begin(), arguments.end());
	}

	Node* FunctionCall::fold()
	{
		for(auto& child : arguments) Helper::folder(child);
		return this;
	}

	bool FunctionCall::check() const
	{
		if (auto* res = table->lookup(identifier))
		{
			if (not res->isInitialized)
			{
				std::cout << SemanticError("function "+identifier+" declared but not yet defined", line, column);
				return false;
			}
			if (not res->type->isFunctionType())
			{
				std::cout << SemanticError("calling non function object: "+identifier, line, column);
				return false;
			}

			const auto& func = res->type->getFunctionType();
			if (not func.variadic and func.parameters.size()!=arguments.size())
			{
				std::cout << WrongArgumentCount(identifier, func.parameters.size(), arguments.size(), line, column);
				return false;
			}

			bool error = false;
			for (size_t i = 0; i< func.parameters.size(); i++)
			{
				error &= Type::convert(arguments[i]->type(), func.parameters[i], false, line, column, true);
			}
			return not error;
		}
		else
		{
			std::cout << UndeclaredError(identifier, line, column);
			return false;
		}
	}

	Type* FunctionCall::type() const
	{
		if (auto* res = table->lookup(identifier))
		{
			if (res->type->isFunctionType())
			{
				return res->type->getFunctionType().returnType;
			}
			else return new Type;
		}
		else
		{
			throw InternalError("function with name: "+identifier+" not found in table");
		}
	}

	bool FunctionCall::constant() const
	{
		return false;
	}

	void FunctionCall::visit(IRVisitor& visitor)
	{
		visitor.visitFunctionCall(*this);
	}

	std::string SubscriptExpr::name() const
	{
		return lhs->name() + "[]";
	}

	std::string SubscriptExpr::value() const
	{
		return lhs->value()+'['+rhs->value()+']';
	}

	std::vector<Node*> SubscriptExpr::children() const
	{
		return {};
	}

	Node* SubscriptExpr::fold()
	{
		Helper::folder(lhs);
		Helper::folder(rhs);
		return this;
	}

	bool SubscriptExpr::check() const
	{
		if (not rhs->type()->isIntegralType())
		{
			std::cout << SemanticError("index type is not integral", line, column);
			return false;
		}
		return true;
	}

	Type* SubscriptExpr::type() const
	{
		return lhs->type()->getDerefType();
	}

	bool SubscriptExpr::constant() const
	{
		return false;
	}

	void SubscriptExpr::visit(IRVisitor& visitor)
	{
		visitor.visitSubscriptExpr(*this);
	}
} // namespace Ast