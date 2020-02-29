#include <iostream>
#include <fstream>
#include <filesystem>

#include "antlr4-runtime.h"
#include "gen/JSONLexer.h"
#include "gen/JSONParser.h"
#include <gen/JSONVisitor.h>

struct DotVisitor : public JSONVisitor
{
    explicit DotVisitor(std::filesystem::path input) : path(std::move(input))
    {
        file = std::ofstream(path.replace_extension("dot"));
        if(not file.is_open())
            throw std::runtime_error("could not open file with path: " + path.string());

        file << "digraph G{\n";
    }
    ~DotVisitor() final
    {
        const auto dot = path.replace_extension("dot");
        const auto png = path.replace_extension("png");

        file << "}" << std::flush;
        std::cerr << system(("dot -Tpng " + dot.string() + " -o " + png.string()).c_str()) << '\n';
    }

    antlrcpp::Any visitJson(JSONParser::JsonContext* context) override
    {
        writeToFile("json: ", context);
        return visitChildren(context);
    }
    antlrcpp::Any visitObject(JSONParser::ObjectContext* context) override
    {
        writeToFile("object: ",context);
        return visitChildren(context);
    }
    antlrcpp::Any visitPair(JSONParser::PairContext* context) override
    {
        writeToFile("pair: ",context);
        return visitChildren(context);
    }
    antlrcpp::Any visitArray(JSONParser::ArrayContext* context) override
    {
        writeToFile("array: ",context);
        return visitChildren(context);
    }
    antlrcpp::Any visitValue(JSONParser::ValueContext* context) override
    {
        writeToFile("value: ", context);
        return visitChildren(context);
    }

    void writeToFile(const std::string& name, antlr4::ParserRuleContext* context)
    {
        if(context->parent != nullptr)
        {
            file << '"' << context->parent << "\" -> \"" << context << "\";\n";
        }

        auto str = context->getText();
        std::replace( str.begin(), str.end(), '"', '\'');
        file << '"' << context << '"' << "[label=\"" + name + str + "\"];\n";
    }

    std::ofstream file;
    std::filesystem::path path;
};

int main(int argc, const char* argv[])
{
    std::ifstream stream("examples/json.json");

    antlr4::ANTLRInputStream input(stream);
    JSONLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);
    JSONParser parser(&tokens);

    auto* tree = parser.value();

    std::filesystem::create_directory("output");
    DotVisitor visitor("output/test");
    auto scene = visitor.visitValue(tree);

    return 0;
}