#include <iostream>
#include <filesystem>
#include <antlr4-runtime.h>

#include <CLexer.h>
#include <CParser.h>
#include <CVisitor.h>


struct DotVisitor : public CVisitor
{
    explicit DotVisitor(std::filesystem::path path) : path(std::move(path))
    {
        std::filesystem::create_directory(this->path.parent_path());
        stream = std::ofstream(this->path.string() + ".dot");
        if(not stream.is_open()) throw std::runtime_error("could not open file: " + this->path.string() + ".dot");

        stream << "digraph G\n";
        stream << "{\n";
    }
    ~DotVisitor() final
    {
        stream << "\"0\"[style = invis];\n";
        stream << "}\n" << std::flush;
        stream.close();

        const auto dot = path.string() + ".dot";
        const auto png = path.string() + ".png";

        system(("dot -Tpng " + dot + " -o " + png).c_str());
        std::filesystem::remove(dot);
    }

    antlrcpp::Any visitBasicExpression(CParser::BasicExpressionContext* context) override
    {
        linkWithParent(context, "basic");
        return visitChildren(context);
    }
    antlrcpp::Any visitUnaryExpression(CParser::UnaryExpressionContext* context) override
    {
        linkWithParent(context, "unary");
        return visitChildren(context);
    }
    antlrcpp::Any visitMultiplicativeExpression(CParser::MultiplicativeExpressionContext* context) override
    {
        linkWithParent(context, "multiplicative");
        return visitChildren(context);
    }
    antlrcpp::Any visitAdditiveExpression(CParser::AdditiveExpressionContext* context) override
    {
        linkWithParent(context, "additive");
        return visitChildren(context);
    }
    antlrcpp::Any visitRelationalExpression(CParser::RelationalExpressionContext* context) override
    {
        linkWithParent(context, "relational");
        return visitChildren(context);
    }
    antlrcpp::Any visitEqualityExpression(CParser::EqualityExpressionContext* context) override
    {
        linkWithParent(context, "equality");
        return visitChildren(context);
    }
    antlrcpp::Any visitAndExpression(CParser::AndExpressionContext* context) override
    {
        linkWithParent(context, "and");
        return visitChildren(context);
    }
    antlrcpp::Any visitOrExpression(CParser::OrExpressionContext* context) override
    {
        linkWithParent(context, "or");
        return visitChildren(context);
    }
    antlrcpp::Any visitExpression(CParser::ExpressionContext* context) override
    {
        linkWithParent(context, "expression");
        return visitChildren(context);
    }
    antlrcpp::Any visitFile(CParser::FileContext* context) override
    {
        linkWithParent(context, "file");
        return visitChildren(context);
    }

    antlrcpp::Any visitTerminal(antlr4::tree::TerminalNode* node)
    {
        linkWithParent(node, node->getText());
        return defaultResult();
    }

    void linkWithParent(antlr4::tree::ParseTree* context, const std::string& name)
    {
        stream << '"' << context->parent << "\" -> \"" << context << "\";\n";
        stream << '"' << context << "\"[label=\"" + name + "\"]";
    }

    std::ofstream stream;
    std::filesystem::path path;
};

void runForExample(const std::filesystem::path& path)
{
    std::ifstream stream(path);
    antlr4::ANTLRInputStream input(stream);
    CLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);
    CParser parser(&tokens);

    const auto string = path.string();
    const auto begin = string.find_first_of('/');
    const auto end = string.find_last_of('.');

    if(begin == std::string::npos or end == std::string::npos)
        throw std::runtime_error("malformed path: " + string);

    const std::filesystem::path output = "output";
    std::filesystem::create_directory(output);

    DotVisitor visitor(output / string.substr(begin + 1, end - begin - 1));
    visitor.visitFile(parser.file());
}

void runForAllExamples(const std::filesystem::path& path = "examples")
{
    for(const auto& entry : std::filesystem::directory_iterator(path))
    {
        if(entry.is_directory())
        {
            runForAllExamples(entry.path());
        }
        else if(entry.is_regular_file())
        {
            runForExample(entry.path());
        }
        else
        {
            std::cerr << "unknown file type in examples: " + entry.path().string() << '\n';
        }
    }
}


int main(int argc, const char** argv)
{
    runForAllExamples();
    return 0;
}