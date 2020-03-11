#include "dotVisitor.h"
#include "visitor.h"
#include "folding.h"
#include "CLexer.h"
#include "CParser.h"


std::filesystem::path swapTopFolder(const std::filesystem::path& path, const std::string& newName) {
	const auto string = path.string();
	const auto begin = string.find_first_of('/');
	const auto end = string.find_last_of('.');

	if (begin == std::string::npos or end == std::string::npos)
		throw std::runtime_error("malformed path: " + string);

	return std::filesystem::path("output") / string.substr(begin + 1, end - begin - 1);
}

void runTest(const std::filesystem::path& path, bool redoExisting) {
	const auto output = swapTopFolder(path, "output");
	if (redoExisting or not std::filesystem::exists(output.string() + ".png")) {
		std::ifstream stream(path);
		antlr4::ANTLRInputStream input(stream);
		CLexer lexer(&input);
		antlr4::CommonTokenStream tokens(&lexer);
		CParser parser(&tokens);

		std::filesystem::create_directory("output");
		DotVisitor dotVisitor(output, &parser.getRuleNames());
		dotVisitor.visit(parser.file());
	}
}

void runTests(const std::filesystem::path& path, bool redoExisting) {
	for (const auto& entry : std::filesystem::directory_iterator(path)) {
		if (entry.is_directory()) {
			runTests(entry.path(), redoExisting);
		} else if (entry.is_regular_file()) {
			runTest(entry.path(), redoExisting);
		} else {
			std::cerr << "unknown file type in examples: " + entry.path().string() << '\n';
		}
	}
}


int main(int argc, const char** argv) {
//	runTests("tests", true);

//	std::stringstream stream("1+1+1;");
	std::ifstream stream("tests/expressions/addmul.c");
	antlr4::ANTLRInputStream input(stream);
	CLexer lexer(&input);
	antlr4::CommonTokenStream tokens(&lexer);
	CParser parser(&tokens);

//	DotVisitor dotVisitor("output/TEST", &parser.getRuleNames());
//	dotVisitor.visit(parser.file());

	const auto root = visitFile(parser.file());
	std::ofstream file("text.dot");
    file << "digraph G\n";
    file << "{\n";
	file << root;
    file << "}\n" << std::flush;
    system(("dot -Tpng " + std::string("text.dot") + " -o " + std::string("text.png")).c_str());
	return 0;
}