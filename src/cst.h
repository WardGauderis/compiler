//============================================================================
// @author      : Thomas Dooms & Ward Gauderis
// @date        : 3/10/20
// @copyright   : BA2 Informatica - Thomas Dooms & Ward Gauderis - University of Antwerp
//============================================================================

#pragma once

#include <CLexer.h>
#include <CParser.h>
#include <antlr4-runtime.h>
#include <filesystem>

class DotVisitor : antlr4::tree::AbstractParseTreeVisitor
{
public:
    DotVisitor(std::ofstream& stream, const std::vector<std::string>& names);

    using antlr4::tree::AbstractParseTreeVisitor::visit;

private:
    antlrcpp::Any visitChildren(antlr4::tree::ParseTree* node) override;

    antlrcpp::Any visitTerminal(antlr4::tree::TerminalNode* node) override;

    void linkWithParent(antlr4::tree::ParseTree* context, const std::string& name);

    std::ofstream& stream;
    const std::vector<std::string>& names;
};

namespace Cst
{
struct Root
{
    explicit Root(std::ifstream& stream)
        : input(stream),
          lexer(&input),
          tokens(&lexer),
          parser(&tokens),
          block(parser.block()),
          rulenames(parser.getRuleNames())
    {
        antlr4::BaseErrorListener
    }

    friend std::ofstream& operator<<(std::ofstream& stream, const std::unique_ptr<Root>& root);

    antlr4::ANTLRInputStream input;
    CLexer lexer;
    antlr4::CommonTokenStream tokens;
    CParser parser;

    CParser::BlockContext* block;
    std::vector<std::string> rulenames;
};

} // namespace Cst