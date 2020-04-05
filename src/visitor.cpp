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
    {
        return {};
    }
}
} // namespace

//============================================================================

Ast::Expr* visitLiteral(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table)
{
    auto* terminal = dynamic_cast<antlr4::tree::TerminalNode*>(context->children[0]);
    if(terminal == nullptr)
    {
        throw UnexpectedContextType(context);
    }


    const auto [line, column] = getColumnAndLine(context);
    auto str = context->getText();
    switch(terminal->getSymbol()->getType())
    {
    case CParser::FLOAT:
        try
        {
            return new Ast::Literal(std::stof(str), table, line, column);
        }
        catch(const std::out_of_range& ex)
        {
            std::cout << LiteralOutOfRange(str, line, column);
            return new Ast::Literal(std::numeric_limits<float>::infinity(), table, line, column);
        }
    case CParser::INT:
        try
        {
            return new Ast::Literal(std::stoi(str), table, line, column);
        }
        catch(const std::out_of_range& ex)
        {
            std::cout << LiteralOutOfRange(str, line, column);
            return new Ast::Literal(std::numeric_limits<int>::max(), table, line, column);
        }
    case CParser::CHAR:
        return new Ast::Literal(str[1], table, line, column);
    case CParser::STRING:
        str.erase(std::remove(str.begin(), str.end(), '"'), str.end());
        return new Ast::StringLiteral(str, table, line, column);
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
    {
        throw UnexpectedContextType(context);
    }
}

Ast::Expr* visitBasicExpr(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table)
{
    if(context->children.size() == 1)
    {
        return visitLiteralOrVariable(context->children[0], table);
    }
    else if(context->children.size() == 3)
    {
        return visitExpr(context->children[1], table);
    }
    else throw InternalError("unknown children size in basic expr");
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

Ast::Expr* visitPostfixExpr(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table)
{
    const auto [line, column] = getColumnAndLine(context);
    if(context->children.size() == 1)
    {
        return visitBasicExpr(context->children[0], table);
    }
    else if(context->children.size() == 2)
    {
        const auto expr = visitPostfixExpr(context->children[0], table);
        return new Ast::PostfixExpr(context->children[1]->getText(), expr, table, line, column);
    }
    else if(context->children.size() == 3)
    {
        auto name = context->children[0]->getText();
        return new Ast::FunctionCall({}, std::move(name), table, line, column);
    }
    else if(context->children.size() == 4)
    {
        if(dynamic_cast<CParser::ArgumentListContext*>(context->children[2]))
        {
            auto args = visitArgumentList(context->children[2], table);
            auto name = context->children[0]->getText();
            return new Ast::FunctionCall(std::move(args), std::move(name), table, line, column);
        }
        else
        {
            auto lhs = visitPostfixExpr(context->children[0], table);
            auto rhs = visitExpr(context->children[2], table);
            return new Ast::SubscriptExpr(lhs, rhs, table, line, column);
        }
    }
    else throw InternalError("unknown children size in postfix expr");
}

Ast::Expr* visitPrefixExpr(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table)
{
    const auto [line, column] = getColumnAndLine(context);
    if(context->children.size() == 1)
    {
        return visitPostfixExpr(context->children[0], table);
    }
    else if(context->children.size() == 2)
    {
        auto* rhs = visitPrefixExpr(context->children[1], table);
        return new Ast::PrefixExpr(context->children[0]->getText(), rhs, table, line, column);
    }
    else if(context->children.size() == 4)
    {
        const auto type = visitTypeName(context->children[1]);
        const auto rhs = visitPrefixExpr(context->children[3], table);
        return new Ast::CastExpr(type, rhs, table, line, column);
    }
    else throw InternalError("unknown children size in prefix expr");
}

Ast::Expr* visitMultiplicativeExpr(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table)
{
    if(context->children.size() == 1)
    {
        return visitPrefixExpr(context->children[0], table);
    }
    else if(context->children.size() == 3)
    {
        const auto [line, column] = getColumnAndLine(context);
        const auto lhs = visitMultiplicativeExpr(context->children[0], table);
        const auto rhs = visitPrefixExpr(context->children[2], table);
        return new Ast::BinaryExpr(context->children[1]->getText(), lhs, rhs, table, line, column);
    }
    else throw InternalError("unknown children size in multiplicative expr");
}

Ast::Expr* visitAdditiveExpr(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table)
{
    if(context->children.size() == 1)
    {
        return visitMultiplicativeExpr(context->children[0], table);
    }
    else if(context->children.size() == 3)
    {
        const auto [line, column] = getColumnAndLine(context);
        const auto lhs = visitAdditiveExpr(context->children[0], table);
        const auto rhs = visitMultiplicativeExpr(context->children[2], table);
        return new Ast::BinaryExpr(context->children[1]->getText(), lhs, rhs, table, line, column);
    }
    else throw InternalError("unknown children size in additive expr");
}

Ast::Expr* visitRelationalExpr(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table)
{
    if(context->children.size() == 1)
    {
        return visitAdditiveExpr(context->children[0], table);
    }
    else if(context->children.size() == 3)
    {
        const auto [line, column] = getColumnAndLine(context);
        const auto lhs = visitRelationalExpr(context->children[0], table);
        const auto rhs = visitAdditiveExpr(context->children[2], table);

        return new Ast::BinaryExpr(context->children[1]->getText(), lhs, rhs, table, line, column);
    }
    else throw InternalError("unknown children size in relational expr");
}

Ast::Expr* visitEqualityExpr(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table)
{
    if(context->children.size() == 1)
    {
        return visitRelationalExpr(context->children[0], table);
    }
    else if(context->children.size() == 3)
    {
        const auto [line, column] = getColumnAndLine(context);
        const auto lhs = visitEqualityExpr(context->children[0], table);
        const auto rhs = visitRelationalExpr(context->children[2], table);
        return new Ast::BinaryExpr(context->children[1]->getText(), lhs, rhs, table, line, column);
    }
    else throw InternalError("unknown children size in equality expr");
}

Ast::Expr* visitAndExpr(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table)
{
    if(context->children.size() == 1)
    {
        return visitEqualityExpr(context->children[0], table);
    }
    else if(context->children.size() == 3)
    {
        const auto [line, column] = getColumnAndLine(context);
        const auto lhs = visitAndExpr(context->children[0], table);
        const auto rhs = visitEqualityExpr(context->children[2], table);
        return new Ast::BinaryExpr("&&", lhs, rhs, table, line, column);
    }
    else throw InternalError("unknown children size in or expr");
}

Ast::Expr* visitOrExpr(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table)
{
    if(context->children.size() == 1)
    {
        return visitAndExpr(context->children[0], table);
    }
    else if(context->children.size() == 3)
    {
        const auto [line, column] = getColumnAndLine(context);
        const auto lhs = visitOrExpr(context->children[0], table);
        const auto rhs = visitAndExpr(context->children[2], table);
        return new Ast::BinaryExpr("||", lhs, rhs, table, line, column);
    }
    else throw InternalError("unknown children size in or expr");
}

Ast::Expr* visitAssignExpr(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table)
{
    if(context->children.size() == 1)
    {
        return visitOrExpr(context->children[0], table);
    }
    else if(context->children.size() == 3)
    {
        const auto [line, column] = getColumnAndLine(context);
        const auto lhs = visitPrefixExpr(context->children[0], table);
        const auto rhs = visitAssignExpr(context->children[2], table);
        return new Ast::Assignment(lhs, rhs, table, line, column);
    }
    else throw InternalError("unknown children size in assign expr");
}

Ast::Expr* visitExpr(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table)
{
    return visitAssignExpr(context->children[0], table);
}

size_t visitSizeExpr(antlr4::tree::ParseTree* context)
{
    auto table = std::make_shared<SymbolTable>(ScopeType::plain);
    auto expr = visitExpr(context, table);
    if( not expr->constant())
    {
        std::cout << SemanticError("expression in array type should have constexpr size");
        throw CompilationError("could not create ast because of above reasons");
    }
    else if(not expr->type()->isIntegralType())
    {
        std::cout << SemanticError("expression in array type should be integral");
        throw CompilationError("could not create ast because of above reasons");
    }
    else
    {
        if(auto* res = dynamic_cast<Ast::Literal*>(expr->fold()))
        {
            if(res->literal.index() == static_cast<size_t>(BaseType::Int))
            {
                return std::get<static_cast<size_t>(BaseType::Int)>(res->literal);
            }
            else if(res->literal.index() == static_cast<size_t>(BaseType::Char))
            {
                return  std::get<static_cast<size_t>(BaseType::Char)>(res->literal);
            }
            else throw InternalError("problems retrieving size from literal");
        }
        else throw InternalError("constexpr size of array could not be folded");
    }
}

Type* visitTypeName(antlr4::tree::ParseTree* context)
{
    const auto type = visitBasicType(context->children[0]);

    if(context->children.size() == 1)
    {
        return type;
    }
    else if(context->children.size() == 2)
    {
        return visitPointerType(context->children[1], type);
    }
    else throw;
}

Type* visitBasicType(antlr4::tree::ParseTree* context)
{
    bool isConst = context->children.size() != 1;
    auto* specifier
    = *std::find_if(context->children.begin(), context->children.end(), [](const auto& context) {
          return typeid(*context) == typeid(CParser::SpecifierContext);
      });
    return new Type(isConst, specifier->getText());
}

Type* visitPointerType(antlr4::tree::ParseTree* context, Type* type)
{
    if(context->children.size() == 1)
    {
        return new Type(false, type);
    }
    else if(context->children.size() == 3)
    {
        return visitPointerType(context->children[2], new Type(true, type));
    }
    else if(dynamic_cast<CParser::QualifierContext*>(context->children[1]))
    {
        return new Type(true, type);
    }
    else
    {
        return visitPointerType(context->children[1], new Type(false, type));
    }
}

Type* visitDeclarationArray(antlr4::tree::ParseTree* context, Type* type)
{
    if(context->children.empty())
    {
        return type;
    }
    else if(context->children.size() == 4)
    {
        auto* temp = visitDeclarationArray(context->children[0], type);
        return new Type(false, visitSizeExpr(context->children[2]), temp);
    }
    else throw InternalError("wrong children size for parameter array");
}

Type* visitParameterArray(antlr4::tree::ParseTree* context, Type* type)
{
    if(context->children.empty())
    {
        return type;
    }
    else if(context->children.size() == 3)
    {
        auto* temp = visitParameterArray(context->children[0], type);
        return new Type(false, 0, temp);
    }
    else if(context->children.size() == 4)
    {
        auto* temp = visitParameterArray(context->children[0], type);
        return new Type(false, visitSizeExpr(context->children[2]), temp);
    }
    else throw InternalError("wrong children size for parameter array");
}

Ast::Statement* visitVariableDeclaration(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table)
{
    const auto [line, column] = getColumnAndLine(context);
    const auto type = visitTypeName(context->children[0]);
    const auto name = context->children[1]->getText();

    if(context->children.size() == 2)
    {
        return new Ast::VariableDeclaration(type, name, nullptr, table, line, column);
    }
    else if(context->children.size() == 3)
    {
        auto temp = visitDeclarationArray(context->children[2], type);
        return new Ast::VariableDeclaration(temp, name, nullptr, table, line, column);
    }
    else if(context->children.size() == 4)
    {
        auto* expr = visitAssignExpr(context->children[3], table);
        return new Ast::VariableDeclaration(type, name, expr, table, line, column);
    }
    else throw InternalError("unknown children size for declaration");
}

std::vector<std::pair<Type*, std::string>> visitDeclarationParameterList(antlr4::tree::ParseTree* context)
{
    const auto type = visitTypeName(context->children[0]);

    if(context->children.size() == 2)
    {
        return {std::make_pair(type, "")};
    }
    if(context->children.size() == 3)
    {
        return {std::make_pair(type, context->children[1]->getText())};
    }
    if(context->children.size() == 4)
    {
        auto res = visitDeclarationParameterList(context->children[3]);
        res.emplace_back(std::make_pair(type, ""));
        return res;
    }
    else if(context->children.size() == 5)
    {
        auto res = visitDeclarationParameterList(context->children[4]);
        res.emplace_back(std::make_pair(type, context->children[1]->getText()));
        return res;
    }
    else throw InternalError("unknown children size for declaration param list");
}

std::vector<std::pair<Type*, std::string>> visitParameterList(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table)
{
    auto type = visitTypeName(context->children[0]);
    auto name = context->children[1]->getText();
    type = visitParameterArray(context->children[2], type);

    if (context->children.size() == 3)
    {
        return { std::make_pair(type, name) };
    }
    else if(context->children.size() == 5)
    {
        auto res = visitParameterList(context->children[4], table);
        res.emplace_back(type, name);
        return res;
    }
    else throw InternalError("parameter list visitor error");
}


Ast::Statement* visitFunctionDeclaration(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table)
{
    const auto [line, column] = getColumnAndLine(context);
    const auto type = visitTypeName(context->children[0]);
    const auto id = context->children[1]->getText();

    std::vector<std::pair<Type*, std::string>> types;
    if(auto* res = dynamic_cast<CParser::DeclarationParameterListContext*>(context->children[3]))
    {
        types = visitDeclarationParameterList(res);
    }
    return new Ast::FunctionDeclaration(type, id, types, table, line, column);
}

Ast::Statement* visitFunctionDefinition(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table)
{
    auto ret = visitTypeName(context->children[0]);

    auto scopeIndex = 4;
    std::vector<std::pair<Type*, std::string>> params;
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

Ast::Statement* visitDeclaration(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table)
{
    if(auto* res = dynamic_cast<CParser::VariableDeclarationContext*>(context->children[0]))
    {
        return visitVariableDeclaration(res, table);
    }
    else if(auto* res = dynamic_cast<CParser::FunctionDeclarationContext*>(context->children[0]))
    {
        return visitFunctionDeclaration(res, table);
    }
    else
    {
        throw InternalError("unknown declaration type");
    }
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

    if(context->children.size() == 5)
    {
        return new Ast::IfStatement(condition, ifBody, nullptr, table, line, column);
    }
    else if(context->children.size() == 7){
        auto* elseBody = visitStatement(context->children[6], table, ScopeType::condition);
        return new Ast::IfStatement(condition, ifBody, elseBody, table, line, column);
    }
    else throw InternalError("unknown child size for if statement");
}

Ast::Statement* visitWhileStatement(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& table)
{
    const auto [line, column] = getColumnAndLine(context);

    if(context->children.size() == 5)
    {
        auto* condition = visitExpr(context->children[2], table);
        auto* body = visitStatement(context->children[4], table, ScopeType::loop);
        return new Ast::LoopStatement(nullptr, condition, nullptr, body, false, table, line, column);
    }
    else if(context->children.size() == 7)
    {
        auto* body = visitStatement(context->children[1], table, ScopeType::loop);
        auto* condition = visitExpr(context->children[4], table);
        return new Ast::LoopStatement(nullptr, condition, nullptr, body, true, table, line, column);
    }
    else throw;
}

Ast::Statement* visitForStatement(antlr4::tree::ParseTree* context, std::shared_ptr<SymbolTable>& parent)
{
    auto table = std::make_shared<SymbolTable>(ScopeType::loop, parent);
    Ast::Statement* init = nullptr;
    Ast::Expr* condition = nullptr;
    Ast::Expr* iteration = nullptr;
    size_t offset = 0;

    if(auto* res = dynamic_cast<CParser::VariableDeclarationContext*>(context->children[2]))
    {
        init = visitVariableDeclaration(res, table);
    }
    else if(auto* res = dynamic_cast<CParser::ExprContext*>(context->children[2]))
    {
        init = visitExpr(res, table);
    }
    else
    {
        offset++;
    }

    if(auto* res = dynamic_cast<CParser::ExprContext*>(context->children[4 - offset]))
    {
        condition = visitExpr(res, table);
    }
    else
    {
        offset++;
    }

    if(auto* res = dynamic_cast<CParser::ExprContext*>(context->children[6 - offset]))
    {
        iteration = visitExpr(res, table);
    }
    else
    {
        offset++;
    }

    auto* body = visitStatement(context->children[8 - offset], table, ScopeType::plain);
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
    else
    {
        throw UnexpectedContextType(context);
    }
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
        else if(auto* res = dynamic_cast<antlr4::tree::TerminalNode*>(child))
        {
            if(res->getSymbol()->getType() == CParser::INCLUDESTDIO)
            {
                statements.emplace_back(new Ast::IncludeStdioStatement(global, line, column));
            }
        }
    }
    return new Ast::Scope(statements, global, line, column);
}

std::unique_ptr<Ast::Node> Ast::from_cst(const std::unique_ptr<Cst::Root>& root, bool fold)
{
    auto res = std::unique_ptr<Ast::Node>(visitFile(root->file));
    res->complete(true, fold, true);
    return res;
}