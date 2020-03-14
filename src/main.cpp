//============================================================================
// @author      : Thomas Dooms & Ward Gauderis
// @date        : 3/10/20
// @copyright   : BA2 Informatica - Thomas Dooms & Ward Gauderis - University of Antwerp
//============================================================================

#include "cst.h"
#include "visitor.h"
#include "folding.h"
#include "CLexer.h"
#include "CParser.h"

std::filesystem::path swap_top_folder(const std::filesystem::path& path, const std::string& new_name)
{
	const auto string = path.string();
	const auto begin = string.find_first_of('/');
	const auto end = string.find_last_of('.');

	if (begin == std::string::npos or end == std::string::npos)
		throw std::runtime_error("malformed path: "+string);

	return std::filesystem::path("output") /
	string.substr(begin+1, end-begin-1);
}

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

    system(("dot -Tpng " + dot.string() + " -o " + png.string()).c_str());
    std::filesystem::remove(dot);
}

void output_all_tests(bool redo_existing)
{
    for(const auto& entry : std::filesystem::recursive_directory_iterator("tests"))
    {
        if(not std::filesystem::is_regular_file(entry)) continue;

        const auto& input = entry.path();

        auto base = swap_top_folder(input, "output");
        const auto name = base.stem().string();
        base = base.parent_path();

        const auto cst_path = base / (name + "-cst.png");
        const auto ast_path = base / (name + "-ast.png");

        std::cout << input << '\n';
        std::cout << cst_path << '\n';
        std::cout << ast_path << '\n';
        std::cout << '\n';

        if(not std::filesystem::exists(cst_path) or not std::filesystem::exists(ast_path))
        {
            std::filesystem::create_directories(base);

            std::ifstream stream(input);
            if(!stream.good()) throw std::runtime_error("problem opening " + input.string());

            const auto cst = std::make_unique<Cst::Root>(stream);
            const auto ast = Ast::from_cst(cst);

            make_dot(cst, cst_path);
            make_dot(ast, ast_path);
        }
    }
}


int main(int argc, const char** argv)
{
//	try
//	{
		output_all_tests(true);
//	}
//	catch (const SyntaxError& ex)
//	{
//		std::cerr << "Syntax Error: " << ex.what() << std::endl;
//	}
//	catch (const SemanticError& ex)
//	{
//		std::cerr << "Semantic Error: " << ex.what() << std::endl;
//	}
//	catch (const WhoopsiePoopsieError& ex)
//	{
//		std::cerr << "Whoopsie Poopsie Error: " << ex.what() << std::endl;
//	}
//	catch (const CompilationError& ex)
//	{
//		std::cerr << "Compilation Error: " << ex.what() << std::endl;
//	}
//	catch (const std::exception& ex)
//	{
//		std::cerr << "Unknown Error: " << ex.what() << std::endl;
//	}
	return 0;
}