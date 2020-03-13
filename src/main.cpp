#include "dotVisitor.h"
#include "visitor.h"
#include "folding.h"
#include "CLexer.h"
#include "CParser.h"

std::filesystem::path swapTopFolder(const std::filesystem::path& path, const std::string& newName)
{
	const auto string = path.string();
	const auto begin = string.find_first_of('/');
	const auto end = string.find_last_of('.');

	if (begin==std::string::npos or end==std::string::npos)
		throw std::runtime_error("malformed path: "+string);

	return std::filesystem::path("output")/string.substr(begin+1, end-begin-1);
}

void runTest(const std::filesystem::path& path, bool redoExisting)
{
	const auto output = swapTopFolder(path, "output");
	if (redoExisting or not std::filesystem::exists(output.string()+".png"))
	{
		std::ifstream stream(path);
		antlr4::ANTLRInputStream input(stream);
		CLexer lexer(&input);
		antlr4::CommonTokenStream tokens(&lexer);
		CParser parser(&tokens);

		std::filesystem::create_directory("output");
		DotVisitor dotVisitor(output, &parser.getRuleNames());
		dotVisitor.visit(parser.block());
	}
}

void runTests(const std::filesystem::path& path, bool redoExisting)
{
	for (const auto& entry : std::filesystem::directory_iterator(path))
	{
		if (entry.is_directory())
		{
			runTests(entry.path(), redoExisting);
		}
		else if (entry.is_regular_file())
		{
			runTest(entry.path(), redoExisting);
		}
		else
		{
			std::cerr << "unknown file type in examples: "+entry.path().string() << '\n';
		}
	}
}

int main(int argc, const char** argv)
{
	try
	{
//		runTests("tests", true);

//		std::stringstream stream("int x = 5; //test");
		std::ifstream stream("tests/assignment/basic.c");
		if(!stream.good()) throw CompilationError("File not found");

		antlr4::ANTLRInputStream input(stream);
		CLexer lexer(&input);
		antlr4::CommonTokenStream tokens(&lexer);
		CParser parser(&tokens);

		antlr4::tree::ParseTree* node = parser.block();

		DotVisitor dotVisitor("output/cst", &parser.getRuleNames());
		dotVisitor.visit(node);

		const auto root = visitBlock(node);
		std::ofstream file("output/ast.dot");
		file << "digraph G\n";
		file << "{\n";
		file << root;
		file << "}\n" << std::flush;
		system(("dot -Tpng "+std::string("output/ast.dot")+" -o "+std::string("output/ast.png")).c_str());
	}
	catch (const SyntaxError& ex)
	{
		std::cerr << "Syntax Error: " << ex.what() << std::endl;
	}
	catch (const SemanticError& ex)
	{
		std::cerr << "Semantic Error: " << ex.what() << std::endl;
	}
	catch (const WhoopsiePoopsieError& ex)
	{
		std::cerr << "Whoopsie Poopsie Error: " << ex.what() << std::endl;
	}
	catch (const CompilationError& ex)
	{
		std::cerr << "Compilation Error: " << ex.what() << std::endl;
	}
	catch (const std::exception& ex)
	{
		std::cerr << "Unknown Error: " << ex.what() << std::endl;
	}
	return 0;
}