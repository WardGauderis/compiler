//
// Created by ward on 3/1/20.
//

#ifndef COMPILER_DOTVISITOR_H
#define COMPILER_DOTVISITOR_H

#include <antlr4-runtime.h>
#include <filesystem>


class DotVisitor : antlr4::tree::AbstractParseTreeVisitor {
public:
	DotVisitor(std::filesystem::path path, const std::vector<std::string>* names);

	~DotVisitor() final;

	using antlr4::tree::AbstractParseTreeVisitor::visit;
private:
	antlrcpp::Any visitChildren(antlr4::tree::ParseTree* node) override;

	antlrcpp::Any visitTerminal(antlr4::tree::TerminalNode* node) override;

	void linkWithParent(antlr4::tree::ParseTree* context, const std::string& name);

	std::ofstream stream;
	std::filesystem::path path;
	const std::vector<std::string>* names;
};


#endif //COMPILER_DOTVISITOR_H
