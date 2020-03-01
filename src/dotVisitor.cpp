//
// Created by ward on 3/1/20.
//

#include "dotVisitor.h"

DotVisitor::DotVisitor(std::filesystem::path path, const std::vector<std::string>* names) : path(std::move(path)),
                                                                                            names(names) {
	std::filesystem::create_directory(this->path.parent_path());
	stream = std::ofstream(this->path.string() + ".dot");
	if (not stream.is_open()) throw std::runtime_error("could not open file: " + this->path.string() + ".dot");

	stream << "digraph G\n";
	stream << "{\n";
}

DotVisitor::~DotVisitor() {
	stream << "\"0\"[style = invis];\n";
	stream << "}\n" << std::flush;
	stream.close();

	const auto dot = path.string() + ".dot";
	const auto png = path.string() + ".png";

	system(("dot -Tpng " + dot + " -o " + png).c_str());
	std::filesystem::remove(dot);
}

void DotVisitor::linkWithParent(antlr4::tree::ParseTree* context, const std::string& name) {
	stream << '"' << context->parent << "\" -> \"" << context << "\";\n";
	stream << '"' << context << "\"[label=\"" + name + "\"]";
}

antlrcpp::Any DotVisitor::visitChildren(antlr4::tree::ParseTree* node) {
	if (auto n = dynamic_cast<antlr4::RuleContext*>(node)) {
		linkWithParent(node, (*names)[n->getRuleIndex()]);
	} else {
		throw std::runtime_error("Geen RuleContext");
	}
	return antlr4::tree::AbstractParseTreeVisitor::visitChildren(node);
}

antlrcpp::Any DotVisitor::visitTerminal(antlr4::tree::TerminalNode* node) {
	linkWithParent(node, node->getText());
	return defaultResult();
}