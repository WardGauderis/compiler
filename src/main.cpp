#include <iostream>
#include <filesystem>
#include <antlr4-runtime.h>

#include "CLexer.h"
#include "CParser.h"
#include "dotVisitor.h"

void runForExample(const std::filesystem::path& path) {
	std::ifstream stream(path);
	antlr4::ANTLRInputStream input(stream);
	CLexer lexer(&input);
	antlr4::CommonTokenStream tokens(&lexer);
	CParser parser(&tokens);
	const auto string = path.string();
	const auto begin = string.find_first_of('/');
	const auto end = string.find_last_of('.');

	if (begin == std::string::npos or end == std::string::npos)
		throw std::runtime_error("malformed path: " + string);

	const std::filesystem::path output = "output";
	std::filesystem::create_directory(output);

	DotVisitor visitor(output / string.substr(begin + 1, end - begin - 1), &parser.getRuleNames());

	visitor.visit(parser.file());
	std::cout << std::endl;
}

void runForAllExamples(const std::filesystem::path& path = "examples") {
	for (const auto& entry : std::filesystem::directory_iterator(path)) {
		if (entry.is_directory()) {
			runForAllExamples(entry.path());
		} else if (entry.is_regular_file()) {
			runForExample(entry.path());
		} else {
			std::cerr << "unknown file type in examples: " + entry.path().string() << '\n';
		}
	}
}


int main(int argc, const char** argv) {
	runForAllExamples();
	return 0;
}