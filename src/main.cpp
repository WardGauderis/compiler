//============================================================================
// @author      : Thomas Dooms & Ward Gauderis
// @date        : 3/10/20
// @copyright   : BA2 Informatica - Thomas Dooms & Ward Gauderis - University of Antwerp
//============================================================================

#include "cst.h"
#include "visitor.h"
#include <boost/program_options.hpp>
#include "IRVisitor/irVisitor.h"
#include "MIPSVisitor/mipsVisitor.h"

std::string CompilationError::file;

template<typename Type>
void make_dot(const Type& elem, const std::filesystem::path& path)
{
	auto dot = path;
	dot.replace_extension("dot");

	auto png = path;
	png.replace_extension("png");

	std::ofstream stream(dot);
	stream << elem;
	stream.close();

	const auto make_png = "dot -Tpng "+dot.string()+" -o "+png.string();
	const auto remove_dot = "rm "+dot.string();
	system(("("+make_png+" ; "+remove_dot+" ) &").c_str());
}

void compileFile(const std::filesystem::path& input, std::filesystem::path output, bool printCst, bool printAst,
		bool optimised)
{
	try
	{
		const auto llPath = output.replace_extension("ll");
		const auto asmPath = output.replace_extension("asm");
		const auto cstPath = output.replace_extension("cst.png");
		output.replace_extension("");
		const auto astPath = output.replace_extension("ast.png");
		CompilationError::file = input;

		std::ifstream stream(input);
		if (!stream.good()) throw CompilationError("file could not be read");

		std::stringstream buffer;
		std::streambuf* old = std::cerr.rdbuf(buffer.rdbuf());
		const auto cst = std::make_unique<Cst::Root>(stream);
		std::cerr.rdbuf(old);
		std::string error = buffer.str();

		if (not error.empty())
		{
			const auto index0 = error.find(':');
			if (index0==std::string::npos) throw SyntaxError("the antlr generated parser had an internal error");

			const auto index1 = error.find(' ', index0);
			if (index1==std::string::npos) throw SyntaxError("the antlr generated parser had an internal error");

			const auto index2 = error.find('\n', index0);
			if (index2==std::string::npos) throw SyntaxError("the antlr generated parser had an internal error");

			try
			{
				const auto line = std::stoi(error.substr(5, index0-5));
				const auto column = std::stoi(error.substr(index0+1, index1-index0-1));
				throw SyntaxError(error.substr(index1+1, index2-index1-1), line, column);
			}
			catch (std::invalid_argument& ex)
			{
				throw InternalError("unexpected outcome of stoi: "+std::string(ex.what()));
			}
		}

		if (printCst) make_dot(cst, cstPath);

		const auto ast = Ast::from_cst(cst);

		if (printAst) make_dot(ast, astPath);

		IRVisitor visitor(input);
		visitor.convertAST(ast);

		if (optimised) visitor.LLVMOptimize();

		visitor.print(llPath);

		MIPSVisitor mVisitor;
		mVisitor.convertIR(visitor.getModule());
		mVisitor.print(asmPath);

		std::cout << "\033[1m" << input.string() << ": \033[1;32mcompilation successful\033[0m\n";
	}
	catch (const SyntaxError& ex)
	{
		std::cout << ex << CompilationError("could not complete compilation due to above errors");
	}
	catch (const InternalError& ex)
	{
		std::cout << ex << CompilationError("could not complete compilation due to above errors");
	}
	catch (const std::exception& ex)
	{
		std::cout << ex.what();
	}

}

std::filesystem::path changeTopFolder(const std::filesystem::path& path, const std::string& new_name)
{
	std::filesystem::path newPath;
	newPath = new_name;
	auto i = path.begin();
	++i;
	while (i!=path.end())
	{
		newPath /= *i;
		++i;
	}
	return newPath;
}

void runTests(const std::filesystem::path& path, bool cst, bool ast, bool optimised)
{
	for (const auto& entry: std::filesystem::recursive_directory_iterator(path))    //TODO file
	{
		if (!entry.is_regular_file()) continue;
		std::filesystem::path newPath = changeTopFolder(entry.path(), "output");
//		std::cout << entry << '\n';
		if (newPath.extension()!=".c") continue;
		std::filesystem::create_directories(newPath.parent_path());
		compileFile(entry.path(), newPath, cst, ast, optimised);
	}
}

namespace po = boost::program_options;

int main(int argc, const char** argv)
{
	std::vector<std::filesystem::path> files;
	po::options_description desc("Compiler usage");
	desc.add_options()
			("help,h", "Display this help message")
			("cst,c", "Print the cst to dot")
			("ast,a", "Print the ast to dot")
			("optimised,o", "Run LLVM optimisation passes")
			("test,t",
					"Compile all files in the given folder recursively and place them in the folder 'output'");
	po::options_description hidden;
	hidden.add_options()
			("files", po::value<std::vector<std::filesystem::path>>(&files), "files to compile");

	po::options_description combined;
	combined.add(desc).add(hidden);

	po::positional_options_description pos;
	pos.add("files", -1);

	po::variables_map vm;
	po::store(po::command_line_parser(argc, argv).options(combined).positional(pos).run(), vm);
	po::notify(vm);

	if (vm.count("help"))
	{
		std::cout << desc;
		return 1;
	}
	if (vm.count("test"))
	{
		if (files.size()!=1 || !std::filesystem::is_directory(files[0]))
		{
			std::cout << desc;
			return 1;
		}
		runTests(files[0], vm.count("cst"), vm.count("ast"), vm.count("optimised"));
		return 0;
	}
	if (!files.empty())
	{
		for (const auto& file: files)
		{
			if (!std::filesystem::is_regular_file(file))
			{
				std::cout << desc;
				return 1;
			}

			for (const auto& file :files)
			{
				compileFile(file, file.filename(), vm.count("cst"), vm.count("ast"), vm.count("optimised"));
			}
			return 0;
		}
	}
	std::cout << desc;
	return 1;
}