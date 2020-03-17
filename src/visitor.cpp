//============================================================================
// @author      : Thomas Dooms & Ward Gauderis
// @date        : 3/10/20
// @copyright   : BA2 Informatica - Thomas Dooms & Ward Gauderis - University of Antwerp
//============================================================================

#include <memory>
#include <tree/ParseTree.h>

#include "CParser.h"
#include "ast.h"
#include "errors.h"
#include "visitor.h"

namespace {
	template<typename Ret>
	struct VisitorHelper {
		VisitorHelper(antlr4::tree::ParseTree* context, std::string name)
				:context(context), name(std::move(name)) { }

		template<typename Func>
		void operator()(size_t size, const Func& func)
		{
			if (size==context->children.size())
			{
				res = func(context);
			}
		}

		Ret* result()
		{
			if (res==nullptr) throw SemanticError("could not find visitor for "+name);
			else return res;
		}

		Ret* res = nullptr;
		antlr4::tree::ParseTree* context;
		std::string name;
	};
}

//============================================================================

Ast::Comment* visitComment(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table)
{
	return new Ast::Comment(context->getText(), table);
}

Ast::Literal* visitLiteral(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table)
{
	auto* terminal = dynamic_cast<antlr4::tree::TerminalNode*>(context->children[0]);
	if (terminal==nullptr) throw InternalError("literal node is not a terminal in the cst");

	switch (terminal->getSymbol()->getType())
	{
	case CParser::FLOAT:
		try
		{
			return new Ast::Literal(std::stof(terminal->getText()), table);
		}
		catch (const std::out_of_range& ex)
		{
			throw SemanticError(
					"float literal '" + terminal->getText()
					+ "' is too large to be represented in a float type");
		}
	case CParser::INT:
		try
		{
			return new Ast::Literal(std::stoi(terminal->getText()), table);
		}
		catch (const std::out_of_range& ex)
		{
			throw SemanticError(
					"integer literal '" + terminal->getText()
					+ "' is too large to be represented in an integer type");
		}
	case CParser::CHAR:
		return new Ast::Literal(terminal->getText()[1], table);
	default:
		throw InternalError("unknown literal type, probably not yet implemented");
	}
}

Ast::Expr* visitLiteralOrVariable(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table)
{
	if (typeid(*context)==typeid(CParser::LiteralContext))
	{
		return visitLiteral(context, table);
	}
	else if (typeid(*context)==typeid(antlr4::tree::TerminalNodeImpl))
	{
	    const auto entry = table->lookup(context->getText());
	    if(not entry.has_value()) throw UndeclaredError(context->getText());
		return new Ast::Variable(entry.value(), table);
	}
	else throw InternalError(std::string("unknown basic expression type: ")+typeid(*context).name());
}

Ast::Expr* visitBasicExpr(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table)
{
	VisitorHelper<Ast::Expr> visitor(context, "basic expression");
	visitor(1, [&](auto* context) -> Ast::Expr*
	{
		return visitLiteralOrVariable(context->children[0], table);
	});
	visitor(3, [&](auto* context)
	{
		return visitExpr(context->children[1], table);
	});
	return visitor.result();
}

Ast::Expr* visitPostfixExpr(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table)
{
	VisitorHelper<Ast::Expr> visitor(context, "postfix expression");
	visitor(1, [&](auto* context)
	{
		return visitBasicExpr(context->children[0], table);
	});
	visitor(2, [&](auto* context)
	{
	    const auto entry = table->lookup(context->children[0]->getText());
         if(not entry.has_value()) throw UndeclaredError(context->getText());
		const auto lhs = new Ast::Variable(entry.value(), table);
		return new Ast::PostfixExpr(context->children[1]->getText(), lhs, table);
	});
	return visitor.result();
}

Ast::Expr* visitprefixExpr(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table)
{
	VisitorHelper<Ast::Expr> visitor(context, "prefix expression");
	visitor(1, [&](auto* context)
	{
		return visitPostfixExpr(context->children[0], table);
	});
	visitor(2, [&](auto* context)
	{
        const auto entry = table->lookup(context->children[1]->getText());
        if(not entry.has_value()) throw UndeclaredError(context->getText());
		const auto rhs = new Ast::Variable(entry.value(), table);
		return new Ast::PrefixExpr(context->children[0]->getText(), rhs, table);
	});
	return visitor.result();
}

Ast::Expr* visitUnaryExpr(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table)
{
	VisitorHelper<Ast::Expr> visitor(context, "unary expression");
	visitor(1, [&](auto* context)
	{
		return visitprefixExpr(context->children[0], table);
	});
	visitor(2, [&](auto* context)
	{
		const auto rhs = visitUnaryExpr(context->children[1], table);
		return new Ast::UnaryExpr(context->children[0]->getText(), rhs, table);
	});
	visitor(4, [&](auto* context)
	{
		const auto type = visitTypeName(context->children[1], table);
		const auto rhs = visitUnaryExpr(context->children[3], table);
		return new Ast::CastExpr(type, rhs, table);
	});
	return visitor.result();
}

Ast::Expr* visitMultiplicativeExpr(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table)
{
	VisitorHelper<Ast::Expr> visitor(context, "multiplicative expression");
	visitor(1, [&](auto* context)
	{
		return visitUnaryExpr(context->children[0], table);
	});
	visitor(3, [&](auto* context)
	{
		const auto lhs = visitMultiplicativeExpr(context->children[0], table);
		const auto rhs = visitUnaryExpr(context->children[2], table);
		return new Ast::BinaryExpr(context->children[1]->getText(), lhs, rhs, table);
	});
	return visitor.result();
}

Ast::Expr* visitAdditiveExpr(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table)
{
	VisitorHelper<Ast::Expr> visitor(context, "additive expression");
	visitor(1, [&](auto* context)
	{
		return visitMultiplicativeExpr(context->children[0], table);
	});
	visitor(3, [&](auto* context)
	{
		const auto lhs = visitAdditiveExpr(context->children[0], table);
		const auto rhs = visitMultiplicativeExpr(context->children[2], table);
		return new Ast::BinaryExpr(context->children[1]->getText(), lhs, rhs, table);
	});
	return visitor.result();
}

Ast::Expr* visitRelationalExpr(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table)
{
	VisitorHelper<Ast::Expr> visitor(context, "relational expression");
	visitor(1, [&](auto* context)
	{
		return visitAdditiveExpr(context->children[0], table);
	});
	visitor(3, [&](auto* context)
	{
		const auto lhs = visitRelationalExpr(context->children[0], table);
		const auto rhs = visitAdditiveExpr(context->children[2], table);
		return new Ast::BinaryExpr(context->children[1]->getText(), lhs, rhs, table);
	});
	return visitor.result();
}

Ast::Expr* visitEqualityExpr(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table)
{
	VisitorHelper<Ast::Expr> visitor(context, "equality expression");
	visitor(1, [&](auto* context)
	{
		return visitRelationalExpr(context->children[0], table);
	});
	visitor(3, [&](auto* context)
	{
		const auto lhs = visitEqualityExpr(context->children[0], table);
		const auto rhs = visitRelationalExpr(context->children[2], table);
		return new Ast::BinaryExpr(context->children[1]->getText(), lhs, rhs, table);
	});
	return visitor.result();
}

Ast::Expr* visitAndExpr(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table)
{
	VisitorHelper<Ast::Expr> visitor(context, "and expression");
	visitor(1, [&](auto* context)
	{
		return visitEqualityExpr(context->children[0], table);
	});
	visitor(3, [&](auto* context)
	{
		const auto lhs = visitAndExpr(context->children[0], table);
		const auto rhs = visitEqualityExpr(context->children[2], table);
		return new Ast::BinaryExpr("&&", lhs, rhs, table);
	});
	return visitor.result();
}

Ast::Expr* visitOrExpr(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table)
{
	VisitorHelper<Ast::Expr> visitor(context, "or expression");
	visitor(1, [&](auto* context)
	{
		return visitAndExpr(context->children[0], table);
	});
	visitor(3, [&](auto* context)
	{
		const auto lhs = visitOrExpr(context->children[0], table);
		const auto rhs = visitAndExpr(context->children[2], table);
		return new Ast::BinaryExpr("||", lhs, rhs, table);
	});
	return visitor.result();
}

Ast::Expr* visitAssignExpr(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table)
{
	VisitorHelper<Ast::Expr> visitor(context, "assign expression");
	visitor(1, [&](auto* context)
	{
		return visitOrExpr(context->children[0], table);
	});
	visitor(3, [&](auto* context)
	{
		const auto identifier = context->children[0]->getText();
		const auto rhs = visitAssignExpr(context->children[2], table);
		const auto entry = table->lookup(identifier);

        if(not entry.has_value()) throw UndeclaredError(identifier);
		auto* var = new Ast::Variable(entry.value(), table);
		return new Ast::Assignment(var, rhs, table);
	});
	return visitor.result();
}

Ast::Expr* visitExpr(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table)
{
	VisitorHelper<Ast::Expr> visitor(context, "expression");
	visitor(1, [&](auto* context)
	{
		return visitAssignExpr(context->children[0], table);
	});
	return visitor.result();
}

Type* visitTypeName(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table)
{
	Type* type = visitBasicType(context->children[0], table);
	VisitorHelper<Type> visitor(context, "expression");
	visitor(1, [&](auto* context)
	{
		return type;
	});
	visitor(2, [&](auto* context)
	{
		return visitPointerType(context->children[1], type, table);
	});
	return visitor.result();
}

Type* visitBasicType(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table)
{
	bool isConst = context->children.size() != 1;

	antlr4::tree::ParseTree* specifier = *std::find_if(context->children.begin(), context->children.end(),
			[](const auto& context)
			{
				return typeid(*context)==typeid(CParser::SpecifierContext);
			});
	return new Type(isConst, specifier->getText());
}

Type* visitPointerType(antlr4::tree::ParseTree* context, Type* type, std::shared_ptr<SymbolTable>& table)
{
	if (context->children.size()==1)
	{
        return new Type(false, type);
	}
	else if (context->children.size()==3)
	{
        return visitPointerType(context->children[2], new Type(true, type), table);
	}
	else if (typeid(*context->children[1])==typeid(CParser::QualifierContext))
	{
        return new Type(true, type);
	}
	else
    {
        return visitPointerType(context->children[1], new Type(false, type), table);
    }
}

Ast::Expr* visitInitializer(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table)
{
	return visitExpr(context->children[0], table);
}

Ast::Statement* visitDeclaration(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table)
{
	Type* type = visitTypeName(context->children[0], table);
	const auto name = context->children[1]->getText();
    const auto entry = table->insert(name, type);
	auto* var = new Ast::Variable(entry, table);

	VisitorHelper<Ast::Statement> visitor(context, "statement");
	visitor(2, [&](auto* context)
	{
		return new Ast::Declaration(var, nullptr, table);
	});
	visitor(4, [&](auto* context)
	{
		auto* expr = visitExpr(context->children[3]->children[0], table);
		return new Ast::Declaration(var, expr, table);
	});
	return visitor.result();
}

Ast::Statement* visitPrintf(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table)
{
	auto* expr = visitExpr(context->children[2], table);
	return new Ast::PrintfStatement(expr, table);
}

Ast::Statement* visitStatement(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table)
{
	const auto child = context->children[0];
	const auto hash = typeid(*child).hash_code();

	if (hash == typeid(CParser::ExprContext).hash_code())
	{
		return visitExpr(child, table);
	}
	else if (hash == typeid(CParser::DeclarationContext).hash_code())
	{
		return visitDeclaration(child, table);
	}
	else if (hash == typeid(CParser::PrintfContext).hash_code())
	{
		return visitPrintf(child, table);
	}
	else throw InternalError(
				std::string("unknown statement type: ") +
				typeid(*context).name() + ":\n\t" + context->getText());
}

std::vector<Ast::Node*> visitBlock(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table)
{
	std::vector<Ast::Node*> nodes;
	for (size_t i = 0; i<context->children.size()-1; i++)
	{
		const auto& child = context->children[i];
		if (typeid(*child)==typeid(CParser::CommentContext))
		{
			nodes.emplace_back(visitComment(child, table));
		}
		else if (typeid(*child)==typeid(CParser::StatementContext))
		{
			nodes.emplace_back(visitStatement(child, table));
		}
		else throw InternalError(std::string("unknown node type: ")+typeid(*child).name());
	}
	return nodes;
}

std::unique_ptr<Ast::Node> Ast::from_cst(const std::unique_ptr<Cst::Root>& root, bool fold)
{
    auto table = std::make_shared<SymbolTable>();
	auto vec = visitBlock(root->block, table);

	auto res = std::make_unique<Ast::Block>(std::move(vec), std::move(table));
	res->complete(true, true, true);
	return res;
}