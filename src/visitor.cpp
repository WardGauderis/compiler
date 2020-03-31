//============================================================================
// @author      : Thomas Dooms & Ward Gauderis
// @date        : 3/10/20
// @copyright   : BA2 Informatica - Thomas Dooms & Ward Gauderis - University of Antwerp
//============================================================================

#include <memory>
#include <tree/ParseTree.h>

#include "CParser.h"
#include "errors.h"
#include "visitor.h"

namespace
{
template <typename Ret>
struct VisitorHelper
{
    VisitorHelper(antlr4::tree::ParseTree* context, std::string name)
    : context(context), name(std::move(name))
    {
    }

    template <typename Func>
    void operator()(size_t size, const Func& func)
    {
        if(size == context->children.size())
        {
            res = func(context);
        }
    }

    Ret result()
    {
        if(res.has_value()) return *res;
        else
            throw InternalError("could not find visitor with "
                                + std::to_string(context->children.size()) + " for " + name);
    }

    std::optional<Ret> res = std::nullopt;
    antlr4::tree::ParseTree* context;
    std::string name;
};

std::pair<size_t, size_t> getColumnAndLine(antlr4::tree::ParseTree* context)
{
    if(auto* res = dynamic_cast<antlr4::ParserRuleContext*>(context))
    {
        return std::make_pair(res->getStart()->getLine(), res->getStart()->getCharPositionInLine());
    }
    else if(auto* res = dynamic_cast<antlr4::tree::TerminalNodeImpl*>(context))
    {
        return std::make_pair(res->symbol->getLine(), res->symbol->getCharPositionInLine());
    }
    else
        return {};
}
} // namespace

//============================================================================

Ast::Comment* visitComment(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table)
{
    const auto [line, column] = getColumnAndLine(context);
    return new Ast::Comment(context->getText(), table, line, column);
}

Ast::Literal* visitLiteral(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table)
{
    auto* terminal = dynamic_cast<antlr4::tree::TerminalNode*>(context->children[0]);
    if(terminal == nullptr) throw UnexpectedContextType(context);

    const auto [line, column] = getColumnAndLine(context);

    switch(terminal->getSymbol()->getType())
    {
    case CParser::FLOAT:
        try
        {
            return new Ast::Literal(std::stof(terminal->getText()), table, line, column);
        }
        catch(const std::out_of_range& ex)
        {
            std::cout << LiteralOutOfRange(terminal->getText(), line, column);
            return new Ast::Literal(std::numeric_limits<float>::infinity(), table, line, column);
        }
    case CParser::INT:
        try
        {
            return new Ast::Literal(std::stoi(terminal->getText()), table, line, column);
        }
        catch(const std::out_of_range& ex)
        {
            std::cout << LiteralOutOfRange(terminal->getText(), line, column);
            return new Ast::Literal(std::numeric_limits<int>::max(), table, line, column);
        }
    case CParser::CHAR:
        return new Ast::Literal(terminal->getText()[1], table, line, column);
    default:
        throw InternalError("unknown literal type, probably not yet implemented", line, column);
    }
}

Ast::Expr* visitLiteralOrVariable(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table)
{
    if(typeid(*context) == typeid(CParser::LiteralContext))
    {
        return visitLiteral(context, table);
    }
    else if(typeid(*context) == typeid(antlr4::tree::TerminalNodeImpl))
    {
        const auto [line, column] = getColumnAndLine(context);
        return new Ast::Variable(context->getText(), table, line, column);
    }
    else
        throw UnexpectedContextType(context);
}

Ast::Expr* visitBasicExpr(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table)
{
    VisitorHelper<Ast::Expr*> visitor(context, "basic expression");
    visitor(1, [&](auto* context) -> Ast::Expr* {
        return visitLiteralOrVariable(context->children[0], table);
    });
    visitor(3, [&](auto* context) { return visitExpr(context->children[1], table); });
    return visitor.result();
}

Ast::Expr* visitPostfixExpr(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table)
{
    const auto [line, column] = getColumnAndLine(context);
    VisitorHelper<Ast::Expr*> visitor(context, "postfix expression");
    visitor(1, [&](auto* context)
    {
        if(auto* res = dynamic_cast<CParser::PrintfContext*>(context->children[0]))
        {
            return visitPrintf(context->children[0], table);
        }
        else return visitBasicExpr(context->children[0], table);
    });
    visitor(2, [&](auto* context)
    {
        const auto identifier = context->children[0]->getText();
        const auto lhs = new Ast::Variable(identifier, table, line, column);
        return new Ast::PostfixExpr(context->children[1]->getText(), lhs, table, line, column);
    });
    visitor(2, [&](auto* context)
    {
      const auto identifier = context->children[0]->getText();
      const auto lhs = new Ast::Variable(identifier, table, line, column);
      return new Ast::PostfixExpr(context->children[1]->getText(), lhs, table, line, column);
    });
    visitor(3, [&](auto* context)
    {
      auto name = context->children[0]->getText();
      return new Ast::FunctionCall({}, std::move(name), table, line, column);
    });
    visitor(4, [&](auto* context)
    {
        auto args = visitArgumentList(context->children[2], table);
        auto name = context->children[0]->getText();
      return new Ast::FunctionCall(std::move(args), std::move(name), table, line, column);
    });
    return visitor.result();
}

Ast::Expr* visitPrefixExpr(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table)
{
    const auto [line, column] = getColumnAndLine(context);
    VisitorHelper<Ast::Expr*> visitor(context, "prefix expression");

    visitor(1, [&](auto* context) { return visitPostfixExpr(context->children[0], table); });
    visitor(2, [&](auto* context) {
        if(typeid(*context->children[1]) == typeid(CParser::PrefixExprContext))
        {
            auto* rhs = visitPrefixExpr(context->children[1], table);
            return new Ast::PrefixExpr(context->children[0]->getText(), rhs, table, line, column);
        }
        else
        {
            const auto identifier = context->children[1]->getText();
            const auto rhs = new Ast::Variable(identifier, table, line, column);

            return new Ast::PrefixExpr(context->children[0]->getText(), rhs, table, line, column);
        }
    });
    visitor(4, [&](auto* context) {
        const auto type = visitTypeName(context->children[1], table);
        const auto rhs = visitPrefixExpr(context->children[3], table);

        return new Ast::CastExpr(type, rhs, table, line, column);
    });
    return visitor.result();
}

Ast::Expr* visitMultiplicativeExpr(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table)
{
    VisitorHelper<Ast::Expr*> visitor(context, "multiplicative expression");
    visitor(1, [&](auto* context) { return visitPrefixExpr(context->children[0], table); });
    visitor(3, [&](auto* context) {
        const auto [line, column] = getColumnAndLine(context);
        const auto lhs = visitMultiplicativeExpr(context->children[0], table);
        const auto rhs = visitPrefixExpr(context->children[2], table);
        return new Ast::BinaryExpr(context->children[1]->getText(), lhs, rhs, table, line, column);
    });
    return visitor.result();
}

Ast::Expr* visitAdditiveExpr(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table)
{
    const auto [line, column] = getColumnAndLine(context);
    VisitorHelper<Ast::Expr*> visitor(context, "additive expression");
    visitor(1, [&](auto* context) { return visitMultiplicativeExpr(context->children[0], table); });
    visitor(3, [&](auto* context) {
        const auto lhs = visitAdditiveExpr(context->children[0], table);
        const auto rhs = visitMultiplicativeExpr(context->children[2], table);
        return new Ast::BinaryExpr(context->children[1]->getText(), lhs, rhs, table, line, column);
    });
    return visitor.result();
}

Ast::Expr* visitRelationalExpr(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table)
{
    VisitorHelper<Ast::Expr*> visitor(context, "relational expression");
    visitor(1, [&](auto* context) { return visitAdditiveExpr(context->children[0], table); });
    visitor(3, [&](auto* context) {
        const auto [line, column] = getColumnAndLine(context);
        const auto lhs = visitRelationalExpr(context->children[0], table);
        const auto rhs = visitAdditiveExpr(context->children[2], table);

        return new Ast::BinaryExpr(context->children[1]->getText(), lhs, rhs, table, line, column);
    });
    return visitor.result();
}

Ast::Expr* visitEqualityExpr(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table)
{
    VisitorHelper<Ast::Expr*> visitor(context, "equality expression");
    visitor(1, [&](auto* context) { return visitRelationalExpr(context->children[0], table); });
    visitor(3, [&](auto* context) {
        const auto [line, column] = getColumnAndLine(context);
        const auto lhs = visitEqualityExpr(context->children[0], table);
        const auto rhs = visitRelationalExpr(context->children[2], table);

        return new Ast::BinaryExpr(context->children[1]->getText(), lhs, rhs, table, line, column);
    });
    return visitor.result();
}

Ast::Expr* visitAndExpr(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table)
{
    VisitorHelper<Ast::Expr*> visitor(context, "and expression");
    visitor(1, [&](auto* context) { return visitEqualityExpr(context->children[0], table); });
    visitor(3, [&](auto* context) {
        const auto [line, column] = getColumnAndLine(context);
        const auto lhs = visitAndExpr(context->children[0], table);
        const auto rhs = visitEqualityExpr(context->children[2], table);

        return new Ast::BinaryExpr("&&", lhs, rhs, table, line, column);
    });
    return visitor.result();
}

Ast::Expr* visitOrExpr(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table)
{
    const auto [line, column] = getColumnAndLine(context);
    VisitorHelper<Ast::Expr*> visitor(context, "or expression");
    visitor(1, [&](auto* context) { return visitAndExpr(context->children[0], table); });
    visitor(3, [&](auto* context) {
        const auto lhs = visitOrExpr(context->children[0], table);
        const auto rhs = visitAndExpr(context->children[2], table);
        return new Ast::BinaryExpr("||", lhs, rhs, table, line, column);
    });
    return visitor.result();
}

Ast::Expr* visitAssignExpr(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table)
{
    VisitorHelper<Ast::Expr*> visitor(context, "assign expression");
    visitor(1, [&](auto* context) { return visitOrExpr(context->children[0], table); });
    visitor(3, [&](auto* context) {
        const auto [line, column] = getColumnAndLine(context);
        const auto identifier = context->children[0]->getText();
        const auto expr = visitAssignExpr(context->children[2], table);

        auto* var = new Ast::Variable(identifier, table, line, column);
        return new Ast::Assignment(var, expr, table, line, column);
    });
    return visitor.result();
}

Ast::Expr* visitExpr(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table)
{
    VisitorHelper<Ast::Expr*> visitor(context, "expression");
    visitor(1, [&](auto* context) { return visitAssignExpr(context->children[0], table); });
    return visitor.result();
}

Type visitTypeName(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table)
{
    const auto [line, column] = getColumnAndLine(context);
    const Type type = visitBasicType(context->children[0], table);

    VisitorHelper<Type> visitor(context, "expression");
    visitor(1, [&](auto* context) { return type; });
    visitor(2, [&](auto* context) { return visitPointerType(context->children[1], type, table); });
    return visitor.result();
}

Type visitBasicType(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table)
{
    bool isConst = context->children.size() != 1;
    antlr4::tree::ParseTree* specifier
    = *std::find_if(context->children.begin(), context->children.end(), [](const auto& context) {
          return typeid(*context) == typeid(CParser::SpecifierContext);
      });
    return Type(isConst, specifier->getText());
}

Type visitPointerType(antlr4::tree::ParseTree* context, Type type, std::shared_ptr<SymbolTable>& table)
{
    Type* ptr = new Type(type);
    if(context->children.size() == 1)
    {
        return Type(false, ptr);
    }
    else if(context->children.size() == 3)
    {
        return visitPointerType(context->children[2], Type(true, ptr), table);
    }
    else if(typeid(*context->children[1]) == typeid(CParser::QualifierContext))
    {
        return Type(true, ptr);
    }
    else
    {
        return visitPointerType(context->children[1], Type(false, ptr), table);
    }
}

Ast::Expr* visitInitializer(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table)
{
    return visitExpr(context->children[0], table);
}

Ast::Statement* visitDeclaration(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table)
{
    const auto [line, column] = getColumnAndLine(context);
    const Type type = visitTypeName(context->children[0], table);
    const auto name = context->children[1]->getText();
    auto* var = new Ast::Variable(name, table, line, column);

    VisitorHelper<Ast::Statement*> visitor(context, "declaration");
    visitor(3, [&](auto* context) {
        return new Ast::Declaration(type, var, nullptr, table, line, column);
    });
    visitor(5, [&](auto* context) {
        auto* expr = visitAssignExpr(context->children[3]->children[0], table);
        return new Ast::Declaration(type, var, expr, table, line, column);
    });
    return visitor.result();
}

Ast::Expr* visitPrintf(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table)
{
    const auto [line, column] = getColumnAndLine(context);
    auto* expr = visitExpr(context->children[2], table);
    return new Ast::PrintfStatement(expr, table, line, column);
}

Ast::Scope* visitScopeStatement(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& parent, ScopeType type)
{
    auto table = std::make_shared<SymbolTable>(type, parent);
    std::vector<Ast::Statement*> statements;
    const auto [line, column] = getColumnAndLine(context);

    for(size_t i = 1; i < context->children.size() - 1; i++)
    {
        if(auto* res = dynamic_cast<CParser::StatementContext*>(context->children[i]))
        {
            auto* statement = visitStatement(res, table, ScopeType::plain);
            if(statement) statements.emplace_back(statement); // needs to check for nullptr
        }
        else if(auto* res = dynamic_cast<CParser::DeclarationContext*>(context->children[i]))
        {
            statements.emplace_back(visitDeclaration(res, table));
        }
    }
    return new Ast::Scope(statements, table, line, column);
}

Ast::Statement* visitIfStatement(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table)
{
    const auto [line, column] = getColumnAndLine(context);

    auto* condition = visitExpr(context->children[2], table);
    auto* ifBody = visitStatement(context->children[4], table, ScopeType::condition);

    VisitorHelper<Ast::Statement*> helper(context, "if");
    helper(5, [&](auto* context)
    {
        return new Ast::IfStatement(condition, ifBody, nullptr, table, line, column);
    });
    helper(7, [&](auto* context) {
        auto* elseBody = visitStatement(context->children[6], table, ScopeType::condition);
        return new Ast::IfStatement(condition, ifBody, elseBody, table, line, column);
    });
    return helper.result();
}

Ast::Statement* visitWhileStatement(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table)
{
    const auto [line, column] = getColumnAndLine(context);

    VisitorHelper<Ast::Statement*> helper(context, "while");
    helper(5, [&](auto* context) {
        auto* condition = visitExpr(context->children[2], table);
        auto* body = visitStatement(context->children[4], table, ScopeType::loop);
        return new Ast::LoopStatement(nullptr, condition, nullptr, body, false, table, line, column);
    });
    helper(7, [&](auto* context) {
        auto* body = visitStatement(context->children[1], table, ScopeType::loop);
        auto* condition = visitExpr(context->children[4], table);
        return new Ast::LoopStatement(nullptr, condition, nullptr, body, true, table, line, column);
    });
    return helper.result();
}

Ast::Statement* visitForStatement(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table)
{
    Ast::Statement* init = nullptr;
    Ast::Expr* condition = nullptr;
    Ast::Expr* iteration = nullptr;
    size_t offset = 0;

    if(auto* res = dynamic_cast<CParser::DeclarationContext*>(context->children[2]))
    {
        init = visitDeclaration(res, table);
    }
    else if(auto* res = dynamic_cast<CParser::ExprContext*>(context->children[2]))
    {
        init = visitExpr(res, table);
    }

    if(auto* res = dynamic_cast<CParser::ExprContext*>(context->children[3]))
    {
        condition = visitExpr(res, table);
    }
    else
    {
        offset++;
    }

    if(auto* res = dynamic_cast<CParser::ExprContext*>(context->children[5 - offset]))
    {
        iteration = visitExpr(res, table);
    }
    else
    {
        offset++;
    }

    auto* body = visitStatement(context->children[7 - offset], table, ScopeType::loop);
    const auto [line, column] = getColumnAndLine(context);

    return new Ast::LoopStatement(init, condition, iteration, body, false, table, line, column);
}

Ast::Expr* visitExprStatement(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table)
{
    if(context->children.size() == 1) return nullptr;
    return visitExpr(context->children[0], table);
}

Ast::Statement* visitControlStatement(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table)
{
    const auto [line, column] = getColumnAndLine(context);
    if(auto* res = dynamic_cast<CParser::ExprStatementContext*>(context->children[1]))
    {
        auto expr = visitExprStatement(res, table);
        return new Ast::ReturnStatement(expr, table, line, column);
    }
    return new Ast::ControlStatement(context->children[0]->getText(), table, line, column);
}

Ast::Statement* visitStatement(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table, ScopeType type)
{
    const auto child = context->children[0];

    if(dynamic_cast<CParser::ExprStatementContext*>(child))
    {
        return visitExprStatement(child, table);
    }
    else if(dynamic_cast<CParser::ControlStatementContext*>(child))
    {
        return visitControlStatement(child, table);
    }
    else if(dynamic_cast<CParser::ScopeStatementContext*>(child))
    {
        return visitScopeStatement(child, table, type);
    }
    else if(dynamic_cast<CParser::IfStatementContext*>(child))
    {
        return visitIfStatement(child, table);
    }
    else if(dynamic_cast<CParser::WhileStatementContext*>(child))
    {
        return visitWhileStatement(child, table);
    }
    else if(dynamic_cast<CParser::ForStatementContext*>(child))
    {
        return visitForStatement(child, table);
    }
    else throw UnexpectedContextType(context);
}

std::vector<std::pair<Type, std::string>> visitParameterList(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table)
{
    auto type = visitTypeName(context->children[0], table);
    auto name = context->children[1]->getText();

    if (context->children.size() == 4)
    {
        auto res = visitParameterList(context->children[3], table);
        res.emplace_back(type, name);
        return res;
    }
    else
    {
        return { std::make_pair(type, name) };
    }
}

std::vector<Ast::Expr*> visitArgumentList(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table)
{
    std::vector<Ast::Expr*> res;
    if(context->children.size() == 3)
    {
        res = visitArgumentList(context->children[2], table);
    }

    auto expr = visitExpr(context->children[0], table);
    res.emplace_back(expr);
    return res;
}

Ast::Statement* visitFunctionDefinition(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table)
{
    auto ret = visitTypeName(context->children[0], table);

    auto scopeIndex = 4;
    std::vector<std::pair<Type, std::string>> params;
    if(auto* res = dynamic_cast<CParser::ParameterListContext*>(context->children[3]))
    {
        params = visitParameterList(res, table);
        scopeIndex = 5;
    }
    auto* body = visitScopeStatement(context->children[scopeIndex], table, ScopeType::function);
    const auto [line, column] = getColumnAndLine(context);
    auto name = context->children[1]->getText();
    return new Ast::FunctionDefinition(ret, std::move(name), params, body, table, line, column);
}

Ast::Scope* visitFile(antlr4::tree::ParseTree* context)
{
    std::vector<Ast::Statement*> statements;
    auto global = std::make_shared<SymbolTable>(ScopeType::global);
    const auto [line, column] = getColumnAndLine(context);

    for(size_t i = 0; i < context->children.size() - 1; i++)
    {
        const auto& child = context->children[i];

        if(dynamic_cast<CParser::DeclarationContext*>(child))
        {
            statements.emplace_back(visitDeclaration(child, global));
        }
        else if(dynamic_cast<CParser::FunctionDefinitionContext*>(child))
        {
            statements.emplace_back(visitFunctionDefinition(child, global));
        }
    }
    return new Ast::Scope(statements, global, line, column);
}

std::unique_ptr<Ast::Node> Ast::from_cst(const std::unique_ptr<Cst::Root>& root, bool fold)
{

    auto res = std::unique_ptr<Ast::Scope>(visitFile(root->file));
    res->complete(true, fold, true);
    return res;
}