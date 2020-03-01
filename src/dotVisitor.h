//============================================================================
// @name        : dot.h
// @author      : Thomas Dooms
// @date        : 3/1/20
// @version     : 
// @copyright   : BA1 Informatica - Thomas Dooms - University of Antwerp
// @description : 
//============================================================================


#pragma once

#include <antlr4-runtime.h>

#include <CLexer.h>
#include <CParser.h>
#include <CVisitor.h>


struct DotVisitor : public CVisitor
{
    explicit DotVisitor(std::filesystem::path path) : path(std::move(path))
    {
        // make sure the folder exists
        if(this->path.has_parent_path())
        {
            std::filesystem::create_directory(this->path.parent_path());
        }

        stream = std::ofstream(this->path.string() + ".dot");

        // make sure the file is open
        if(not stream.is_open())
        {
            throw std::runtime_error("could not open file: " + this->path.string() + ".dot");
        }

        stream << "digraph G\n";
        stream << "{\n";
    }
    ~DotVisitor() final
    {
        stream << "\"0\"[style = invis];\n";
        stream << "}\n" << std::flush;
        stream.close();

        const auto dot = path.string() + ".dot";
        const auto png = path.string() + ".png";

        system(("dot -Tpng " + dot + " -o " + png).c_str());
        std::filesystem::remove(dot);
    }

    antlrcpp::Any visitBasicExpression(CParser::BasicExpressionContext* context) override
    {
        linkWithParent(context, "basic");
        return visitChildren(context);
    }
    antlrcpp::Any visitUnaryExpression(CParser::UnaryExpressionContext* context) override
    {
        linkWithParent(context, "unary");
        return visitChildren(context);
    }
    antlrcpp::Any visitMultiplicativeExpression(CParser::MultiplicativeExpressionContext* context) override
    {
        linkWithParent(context, "multiplicative");
        return visitChildren(context);
    }
    antlrcpp::Any visitAdditiveExpression(CParser::AdditiveExpressionContext* context) override
    {
        linkWithParent(context, "additive");
        return visitChildren(context);
    }
    antlrcpp::Any visitRelationalExpression(CParser::RelationalExpressionContext* context) override
    {
        linkWithParent(context, "relational");
        return visitChildren(context);
    }
    antlrcpp::Any visitEqualityExpression(CParser::EqualityExpressionContext* context) override
    {
        linkWithParent(context, "equality");
        return visitChildren(context);
    }
    antlrcpp::Any visitAndExpression(CParser::AndExpressionContext* context) override
    {
        linkWithParent(context, "and");
        return visitChildren(context);
    }
    antlrcpp::Any visitOrExpression(CParser::OrExpressionContext* context) override
    {
        linkWithParent(context, "or");
        return visitChildren(context);
    }
    antlrcpp::Any visitExpression(CParser::ExpressionContext* context) override
    {
        linkWithParent(context, "expression");
        return visitChildren(context);
    }
    antlrcpp::Any visitFile(CParser::FileContext* context) override
    {
        linkWithParent(context, "file");
        return visitChildren(context);
    }

    antlrcpp::Any visitTerminal(antlr4::tree::TerminalNode* node) override
    {
        linkWithParent(node, node->getText());
        return defaultResult();
    }

    void linkWithParent(antlr4::tree::ParseTree* context, const std::string& name)
    {
        stream << '"' << context->parent << "\" -> \"" << context << "\";\n";
        stream << '"' << context << "\"[label=\"" + name + "\"]";
    }

    std::ofstream stream;
    std::filesystem::path path;
};