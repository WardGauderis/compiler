//============================================================================
// @author      : Thomas Dooms
// @date        : 3/21/20
// @copyright   : BA2 Informatica - Thomas Dooms - University of Antwerp
//============================================================================

#include "node.h"

namespace Ast
{
std::ofstream& operator<< (std::ofstream& stream, const std::unique_ptr<Node>& root)
{
    stream << "digraph G\n";
    stream << "{\n";

    std::function<void (Node*)> recursion = [&] (Node* node) {
        stream << '"' << node << "\"[label=\"" << node->name () << "\\n"
               << node->value () << "\", shape=box, style=filled, color=\"" << node->color () << "\"];\n";

        for (const auto child : node->children ())
        {
            stream << '"' << node << "\" -> \"" << child << "\";\n";
            recursion (child);
        }
    };
    recursion (root.get ());
    stream << "}\n";
    return stream;
}

void Node::complete (bool check, bool fold, bool output)
{
    bool result = true;
    std::function<void (Ast::Node*)> recursion = [&] (Ast::Node* root) {
        result &= root->check ();
        for (const auto child : root->children ())
        {
            recursion (child);
        }
    };
    if (check)
    {
        recursion (this);
    }
    if (not result)
    {
        throw CompilationError ("could not complete compilation due to above errors");
    }
    if (fold)
    {
        [[maybe_unused]] auto _ = this->fold ();
    }
}
} // namespace Ast