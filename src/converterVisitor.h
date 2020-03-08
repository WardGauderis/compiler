//
// Created by ward on 3/8/20.
//

#ifndef COMPILER_CONVERTERVISITOR_H
#define COMPILER_CONVERTERVISITOR_H

#include "CVisitor.h"

class ConverterVisitor: public CVisitor {
public:
	antlrcpp::Any visitBasicExpression(CParser::BasicExpressionContext *ctx) final;

	antlrcpp::Any visitUnaryExpression(CParser::UnaryExpressionContext *ctx) final;

	antlrcpp::Any visitMultiplicativeExpression(CParser::MultiplicativeExpressionContext *ctx) final;

	antlrcpp::Any visitAdditiveExpression(CParser::AdditiveExpressionContext *ctx) final;

	antlrcpp::Any visitRelationalExpression(CParser::RelationalExpressionContext *ctx) final;

	antlrcpp::Any visitEqualityExpression(CParser::EqualityExpressionContext *ctx) final;

	antlrcpp::Any visitAndExpression(CParser::AndExpressionContext *ctx) final;

	antlrcpp::Any visitOrExpression(CParser::OrExpressionContext *ctx) final;

	antlrcpp::Any visitExpression(CParser::ExpressionContext *ctx) final;

	antlrcpp::Any visitFile(CParser::FileContext *ctx) final;
};

#endif //COMPILER_CONVERTERVISITOR_H
