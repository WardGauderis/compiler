//============================================================================
// @author      : Thomas Dooms & Ward Gauderis
// @date        : 3/10/20
// @copyright   : BA2 Informatica - Thomas Dooms & Ward Gauderis - University of Antwerp
//============================================================================

#include "cst.h"
#include "visitor.h"

std::string CompilationError::file;

std::filesystem::path swap_top_folder(const std::filesystem::path& path, const std::string& new_name)
{
    const auto string = path.string();
    const auto begin  = string.find_first_of('/');
    const auto end    = string.find_last_of('.');

    if (begin == std::string::npos or end == std::string::npos)
        throw std::runtime_error("malformed path: " + string);

    return std::filesystem::path("output") / string.substr(begin + 1, end - begin - 1);
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

    const auto make_png   = "dot -Tpng " + dot.string() + " -o " + png.string();
    const auto remove_dot = "rm " + dot.string();
    system(("(" + make_png + " ; " + remove_dot + " ) &").c_str());
}

void output_all_tests(bool redo_existing)
{
    for (const auto& entry : std::filesystem::recursive_directory_iterator("mytests"))
    {
        try
        {
            if (not std::filesystem::is_regular_file(entry)) continue;

            const auto& input = entry.path();

            auto base       = swap_top_folder(input, "output");
            const auto name = base.stem().string();
            base            = base.parent_path();

            const auto cst_path = base / (name + "-cst.png");
            const auto ast_path = base / (name + "-ast.png");

            if (redo_existing or not std::filesystem::exists(cst_path) or not std::filesystem::exists(ast_path))
            {
                CompilationError::file = input;
                std::filesystem::create_directories(base);

                std::ifstream stream(input);
                if (!stream.good()) throw std::runtime_error("problem opening " + input.string());

                std::stringstream buffer;
                std::streambuf* old = std::cerr.rdbuf(buffer.rdbuf());
                const auto cst      = std::make_unique<Cst::Root>(stream);
                std::cerr.rdbuf(old);
                std::string warning = buffer.str();
                if (!warning.empty()) throw SyntaxError(warning.substr(0, warning.size() - 1));

	            make_dot(cst, cst_path);

	            const auto ast = Ast::from_cst(cst, true);

                make_dot(ast, ast_path);

	            Ast::ast2ir(ast, input);
            }
        }
        catch (const std::exception& ex)
        {
            std::cout << ex.what();
        }
    }
}

int main(int argc, const char** argv)
{
    output_all_tests(true);
    return 0;
}