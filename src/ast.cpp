//============================================================================
// @author      : Thomas Dooms & Ward Gauderis
// @date        : 3/10/20
// @copyright   : BA2 Informatica - Thomas Dooms & Ward Gauderis - University of Antwerp
//============================================================================

#include "ast.h"
#include <iostream>
#include <functional>
#include <fstream>

namespace
{
    template<typename Type>
    bool assign_fold(Type*& elem)
    {
        if(auto* res = elem->fold())
        {
            elem = res;
            return true;
        }
        return false;
    }

    template <typename Type0, typename Type1>
    Ast::Literal* fold_modulo(Type0 lhs, Type1 rhs)
    {
        if constexpr (std::is_integral_v<Type0> and std::is_integral_v<Type1>)
        {
            return new Ast::Literal(lhs % rhs);
        }
        else
        {
            throw WhoopsiePoopsieError("Someone should already have checked if "
                                       "modulo is done on floating point before");
        }
    }

    template <typename Type0, typename Type1>
    Ast::Literal* fold_binary(Type0 lhs, Type1 rhs, const std::string& operation)
    {
        if ((operation == "/" or operation == "%") and rhs == 0)
            return nullptr;

        if (operation == "+")
            return new Ast::Literal(lhs + rhs);
        else if (operation == "-")
            return new Ast::Literal(lhs - rhs);
        else if (operation == "*")
            return new Ast::Literal(lhs * rhs);
        else if (operation == "/")
            return new Ast::Literal(lhs / rhs);
        else if (operation == "%")
            return fold_modulo(lhs, rhs);
        else if (operation == "<")
            return new Ast::Literal(lhs < rhs);
        else if (operation == ">")
            return new Ast::Literal(lhs > rhs);
        else if (operation == "<=")
            return new Ast::Literal(lhs <= rhs);
        else if (operation == ">=")
            return new Ast::Literal(lhs >= rhs);
        else if (operation == "==")
            return new Ast::Literal(lhs == rhs);
        else if (operation == "!=")
            return new Ast::Literal(lhs != rhs);
        else if (operation == "&&")
            return new Ast::Literal(lhs && rhs);
        else if (operation == "||")
            return new Ast::Literal(lhs || rhs);
        else
            throw SyntaxError("unknown binary operation");
    }

    template <typename Type>
    Ast::Literal* fold_unary(Type rhs, const std::string& operation)
    {
        if (operation == "+")
            return new Ast::Literal(rhs);
        else if (operation == "-")
            return new Ast::Literal(-rhs);
        else if (operation == "!")
            return new Ast::Literal(!rhs);
        else
            throw SyntaxError("unknown unary operation");
    }
}

namespace Ast {
	std::ofstream& operator<<(std::ofstream& stream, const std::unique_ptr<Node>& root)
	{
		stream << "digraph G\n";
		stream << "{\n";

		std::function<void(Node*)> recursion = [&](Node* node)
		{
			stream << '"' << node << "\"[label=\""
			       << node->name() << "\\n"
			       << node->value() << "\", shape=box, style=filled, color=\""
			       << node->color() << "\"];\n";

			for (const auto child : node->children())
			{
			    if(child == nullptr) std::cout << typeid(*node).name() << '\n';
				stream << '"' << node << "\" -> \"" << child << "\";\n";
				recursion(child);
			}
		};
		recursion(root.get());
		stream << "}\n";
		return stream;
	}

	std::string Expr::color() const
	{
		return "#ced6eb"; // light blue
	}

	std::string Statement::color() const
	{
		return " #ebcee5"; // light orange/pink
	}

	std::string Comment::name() const
	{
		return "comment";
	}

	std::string Comment::value() const
	{
		return "...";
	}

	std::vector<Node*> Comment::children() const
	{
		return {};
	}

	std::string Comment::color() const
	{
		return "#d5ceeb"; // light purple
	}
    Literal* Comment::fold()
    {
	    return nullptr;
    }

	std::string Block::name() const
	{
		return "block";
	}

	std::string Block::value() const
	{
		return "";
	}

	std::vector<Node*> Block::children() const
	{
		return nodes;
	}

	std::string Block::color() const
	{
		return "#ceebe3"; // light green
	}

    Literal* Block::fold()
    {
        for(auto& child : nodes) assign_fold(child);
        return nullptr;
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

	std::vector<Node*> Literal::children() const
	{
		return {};
	}

    Literal* Literal::fold()
    {
        return this;
    }

	std::string Type::name() const
	{
		return "type";
	}

	std::string Type::color() const
	{
		return "";
	}
    Literal* Type::fold()
    {
        return nullptr;
    }

	std::string BasicType::value() const
	{
		return +(isConst ? "const " : "")+type;
	}

	std::vector<Node*> BasicType::children() const
	{
		return {};
	}

	std::string PointerType::value() const
	{
		return baseType->value()+"*"+(isConst ? " const" : "");
	}

	std::vector<Node*> PointerType::children() const
	{
		return {baseType};
	}

	std::string Variable::name() const
	{
		return "variable";
	}

	std::string Variable::value() const
	{
		if (type) return type->value()+" "+identifier;
		return identifier;
	}

	std::vector<Node*> Variable::children() const
	{
		return {};
	}
    Literal* Variable::fold()
    {
        return nullptr;
    }

	std::string BinaryExpr::name() const
	{
		return "binary expression";
	}

	std::string BinaryExpr::value() const
	{
		return operation;
	}

	std::vector<Node*> BinaryExpr::children() const
	{
		return {lhs, rhs};
	}
    Literal* BinaryExpr::fold()
    {
	    auto* new_lhs = lhs->fold();
	    auto* new_rhs = rhs->fold();

	    const auto set_folded = [&]()
	        {
              if(new_lhs) lhs = new_lhs;
              if(new_rhs) rhs = new_rhs;
              return nullptr;
	        };

	    if(new_lhs and new_rhs)
	    {
            const auto lambda = [&](const auto& lhs, const auto& rhs)
            {
              auto* res = fold_binary(lhs, rhs, operation);
              if(res) return res;
              else set_folded();
            };
	        // TODO: deletus feetus, memory leakus
            return std::visit(lambda, new_lhs->literal, new_rhs->literal);
	    }
	    else
        {
	        return set_folded();
        }
    }

	std::string PostfixExpr::name() const
	{
		return "postfix expression";
	}

	std::string PostfixExpr::value() const
	{
		return operation;
	}

	std::vector<Node*> PostfixExpr::children() const
	{
		return {variable};
	}
    Literal* PostfixExpr::fold()
    {
        return nullptr;
    }

	std::string PrefixExpr::name() const
	{
		return "prefix expression";
	}

	std::string PrefixExpr::value() const
	{
		return operation;
	}

	std::vector<Node*> PrefixExpr::children() const
	{
		return {variable};
	}
    Literal* PrefixExpr::fold()
    {
        return nullptr;
    }

	std::string UnaryExpr::name() const
	{
		return "unary expression";
	}

	std::string UnaryExpr::value() const
	{
		return operation;
	}

	std::vector<Node*> UnaryExpr::children() const
	{
		return {operand};
	}
    Literal* UnaryExpr::fold()
    {
	    auto* new_operand = operand->fold();
        const auto lambda = [&](const auto& val)
        {
          return fold_unary(val, operation);
        };

	    if(new_operand) return std::visit(lambda, new_operand->literal);
        return nullptr;
    }

	std::string CastExpr::name() const
	{
		return "cast expression";
	}

	std::string CastExpr::value() const
	{
		return '('+type->value()+')';
	}

	std::vector<Node*> CastExpr::children() const
	{
		return {operand};
	}
    Literal* CastExpr::fold()
    {
        assign_fold(operand);
        return nullptr;
    }

	std::string Assignment::name() const
	{
		return "assignment";
	}

	std::string Assignment::value() const
	{
		return "";
	}

	std::vector<Node*> Assignment::children() const
	{
		return {variable, expr};
	}

    Literal* Assignment::fold()
    {
	    assign_fold(expr);
        return nullptr;
    }

	std::string Declaration::name() const
	{
		return "declaration";
	}

	std::string Declaration::value() const
	{
		return "";
	}

	std::vector<Node*> Declaration::children() const
	{
		if (expr) return {variable, expr};
		else return {variable};
	}
    Literal* Declaration::fold()
    {
        if(expr) assign_fold(expr);
        return nullptr;
    }

	std::string PrintfStatement::name() const
	{
		return "printf";
	}

	std::string PrintfStatement::value() const
	{
		return "";
	}

	std::vector<Node*> PrintfStatement::children() const
	{
		return {expr};
	}
    Literal* PrintfStatement::fold()
    {
        assign_fold(expr);
        return nullptr;
    }

}