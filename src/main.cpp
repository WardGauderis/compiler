#include <iostream>
#include <fstream>

#include "antlr4-runtime.h"
#include "gen/JSONLexer.h"
#include "gen/JSONParser.h"
#include <gen/JSONVisitor.h>

struct Visitor : public JSONVisitor
{
    antlrcpp::Any visitJson(JSONParser::JsonContext* context) override
    {
        std::cout << "json element\n";
        return visitChildren(context);
    }
    antlrcpp::Any visitObject(JSONParser::ObjectContext* context) override
    {
        std::cout << "object element\n";
        return visitChildren(context);
    }
    antlrcpp::Any visitPair(JSONParser::PairContext* context) override
    {
        std::cout << "pair element\n";
        return visitChildren(context);
    }
    antlrcpp::Any visitArray(JSONParser::ArrayContext* context) override
    {
        std::cout << "array element\n";
        return visitChildren(context);
    }
    antlrcpp::Any visitValue(JSONParser::ValueContext* context) override
    {
        std::cout << "value element\n";
        return visitChildren(context);
    }
};

int main(int argc, const char* argv[])
{
    std::ifstream stream("examples/json.json");

    antlr4::ANTLRInputStream input(stream);
    JSONLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);
    JSONParser parser(&tokens);

    auto* tree = parser.value();

    Visitor visitor;
    auto scene = visitor.visitValue(tree);

    return 0;
}