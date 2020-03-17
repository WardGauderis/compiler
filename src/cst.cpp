//============================================================================
// @author      : Thomas Dooms & Ward Gauderis
// @date        : 3/10/20
// @copyright   : BA2 Informatica - Thomas Dooms & Ward Gauderis - University of Antwerp
//============================================================================

#include "cst.h"
#include "errors.h"

DotVisitor::DotVisitor(std::ofstream& stream, const std::vector<std::string>& names)
    : stream(stream), names(names)
{
}

void DotVisitor::linkWithParent(antlr4::tree::ParseTree* context, const std::string& name)
{
    stream << '"' << context->parent << "\" -> \"" << context << "\";\n";
    stream << '"' << context << "\"[label=\"" + name + "\"];\n";
}

antlrcpp::Any DotVisitor::visitChildren(antlr4::tree::ParseTree* node)
{
    if (auto n = dynamic_cast<antlr4::RuleContext*>(node))
    {
        linkWithParent(node, names[n->getRuleIndex()]);
    }
    else
    {
        throw InternalError(std::string("node is not rule context but: ") + typeid(*node).name());
    }
    return antlr4::tree::AbstractParseTreeVisitor::visitChildren(node);
}

antlrcpp::Any DotVisitor::visitTerminal(antlr4::tree::TerminalNode* node)
{
    linkWithParent(node, node->getText());
    return defaultResult();
}

namespace Cst
{
std::ofstream& operator<<(std::ofstream& stream, const std::unique_ptr<Root>& root)
{
    stream << "digraph G\n";
    stream << "{\n";

    DotVisitor visitor(stream, root->rulenames);
    visitor.visit(root->block);

    stream << "}\n";
    return stream;
}
} // namespace Cst
