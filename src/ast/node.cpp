//============================================================================
// @author      : Thomas Dooms
// @date        : 3/21/20
// @copyright   : BA2 Informatica - Thomas Dooms - University of Antwerp
//============================================================================

#include "node.h"
#include "helper.h"

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

void Node::complete ()
{
    bool check_result = true;
    bool fill_result = true;

    std::function<void (Ast::Node*)> recursion = [&] (Ast::Node* root) {
        fill_result &= root->fill();
        check_result &= root->check ();
      for (const auto child : root->children ())
      {
        recursion (child);
      }
    };

    recursion(this);
    if (not check_result or not fill_result)
    {
        throw CompilationError ("could not complete compilation due to above errors");
    }
    [[maybe_unused]] auto _ = this->fold();
}

std::string Node::value() const
{
    return "";
}

std::vector<Node *> Node::children() const
{
    return {};
}

[[nodiscard]] bool Node::fill() const
{
    return true;
}

[[nodiscard]] bool Node::check() const
{
    return true;
}

} // namespace Ast