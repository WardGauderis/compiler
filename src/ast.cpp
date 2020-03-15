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
    Ast::Literal* fold_modulo(Type0 lhs, Type1 rhs, std::shared_ptr<SymbolTable> table)
    {
        if constexpr (std::is_integral_v<Type0> and std::is_integral_v<Type1>)
        {
            return new Ast::Literal(lhs % rhs, std::move(table));
        }
        else
        {
            throw WhoopsiePoopsieError("Someone should already have checked if "
                                       "modulo is done on floating point before");
        }
    }

    template <typename Type0, typename Type1>
    Ast::Literal* fold_binary(Type0 lhs, Type1 rhs, const std::string& operation, std::shared_ptr<SymbolTable> table)
    {
        if ((operation == "/" or operation == "%") and rhs == 0)
            return nullptr;

        if (operation == "+")
            return new Ast::Literal(lhs + rhs, std::move(table));
        else if (operation == "-")
            return new Ast::Literal(lhs - rhs, std::move(table));
        else if (operation == "*")
            return new Ast::Literal(lhs * rhs, std::move(table));
        else if (operation == "/")
            return new Ast::Literal(lhs / rhs, std::move(table));
        else if (operation == "%")
            return fold_modulo(lhs, rhs, std::move(table));
        else if (operation == "<")
            return new Ast::Literal(lhs < rhs, std::move(table));
        else if (operation == ">")
            return new Ast::Literal(lhs > rhs, std::move(table));
        else if (operation == "<=")
            return new Ast::Literal(lhs <= rhs, std::move(table));
        else if (operation == ">=")
            return new Ast::Literal(lhs >= rhs, std::move(table));
        else if (operation == "==")
            return new Ast::Literal(lhs == rhs, std::move(table));
        else if (operation == "!=")
            return new Ast::Literal(lhs != rhs, std::move(table));
        else if (operation == "&&")
            return new Ast::Literal(lhs && rhs, std::move(table));
        else if (operation == "||")
            return new Ast::Literal(lhs || rhs, std::move(table));
        else
            throw SyntaxError("unknown binary operation");
    }

    template <typename Type>
    Ast::Literal* fold_unary(Type operand, const std::string& operation, std::shared_ptr<SymbolTable> table)
    {
        if (operation == "+")
            return new Ast::Literal(operand, std::move(table));
        else if (operation == "-")
            return new Ast::Literal(-operand, std::move(table));
        else if (operation == "!")
            return new Ast::Literal(!operand, std::move(table));
        else
            throw SyntaxError("unknown unary operation");
    }
    template<typename Type>
    Ast::Literal* fold_cast(Type operand, const std::string& operation, std::shared_ptr<SymbolTable> table)
    {
        if(operation == "float")
            return new Ast::Literal((float)operand, std::move(table));
        else if(operation == "double")
            return new Ast::Literal((double)operand, std::move(table));
        else if(operation == "char")
            return new Ast::Literal((char)operand, std::move(table));
        else if(operation == "short")
            return new Ast::Literal((short)operand, std::move(table));
        else if(operation == "int")
            return new Ast::Literal((int)operand, std::move(table));
        else if(operation == "long")
            return new Ast::Literal((long)operand, std::move(table));
        else if(operation.find('*') != std::string::npos)
            return new Ast::Literal((ptr_type)operand, std::move(table));
        else throw WhoopsiePoopsieError("unknown type for conversion: " + operation);
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

	std::string Variable::name() const
	{
		return entry->first;
	}

	std::string Variable::value() const
	{
		return entry->second.first->print();
	}

	std::vector<Node*> Variable::children() const
	{
		return {};
	}
    std::string Variable::color() const
    {
        return "#ebe6ce";
    }
    Literal* Variable::fold()
    {
        const auto& entry = table->lookup(name());
        if(name() == "xxx")
        {
            const auto temp = *table;
            std::cout << '\n';
        }
        if(entry->second.second.has_value())
        {
            return new Ast::Literal(entry->second.second.value(), table);
        }
        else return nullptr;
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

	    const auto set_folded = [&]() -> Ast::Literal*
	        {
              if(new_lhs) lhs = new_lhs;
              if(new_rhs) rhs = new_rhs;
              return nullptr;
	        };

	    if(new_lhs and new_rhs)
	    {
            const auto lambda = [&](const auto& lhs, const auto& rhs)
            {
              auto* res = fold_binary(lhs, rhs, operation, table);
              if(res) return res;
              else return set_folded();
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
          return fold_unary(val, operation, table);
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
		return '(' + type->print()+')';
	}

	std::vector<Node*> CastExpr::children() const
	{
		return {operand};
	}
    Literal* CastExpr::fold()
    {
        auto* new_operand = operand->fold();
        const auto lambda = [&](const auto& val)
        {
          return fold_cast(val, type->print(), table);
        };

        if(new_operand) return std::visit(lambda, new_operand->literal);
        else return nullptr;
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
	    if(not expr) return nullptr;

        if(auto* res = expr->fold())
        {
            if(variable->entry->second.first->isConst)
            {
                table->set_literal(variable->name(), res->literal);
            }
            expr = res;
        }
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