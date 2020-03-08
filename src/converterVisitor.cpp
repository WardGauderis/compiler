//
// Created by ward on 3/8/20.
//

#include "converterVisitor.h"

antlrcpp::Any ConverterVisitor::visitBasicExpression(CParser::BasicExpressionContext* ctx) {
	return visitChildren(ctx);
}

antlrcpp::Any ConverterVisitor::visitUnaryExpression(CParser::UnaryExpressionContext* ctx) {
	return visitChildren(ctx);
}

antlrcpp::Any ConverterVisitor::visitMultiplicativeExpression(CParser::MultiplicativeExpressionContext* ctx) {
	return visitChildren(ctx);
}

antlrcpp::Any ConverterVisitor::visitAdditiveExpression(CParser::AdditiveExpressionContext* ctx) {
	return visitChildren(ctx);
}

antlrcpp::Any ConverterVisitor::visitRelationalExpression(CParser::RelationalExpressionContext* ctx) {
	return visitChildren(ctx);
}

antlrcpp::Any ConverterVisitor::visitEqualityExpression(CParser::EqualityExpressionContext* ctx) {
	return visitChildren(ctx);
}

antlrcpp::Any ConverterVisitor::visitAndExpression(CParser::AndExpressionContext* ctx) {
	return visitChildren(ctx);
}

antlrcpp::Any ConverterVisitor::visitOrExpression(CParser::OrExpressionContext* ctx) {
	return visitChildren(ctx);
}

antlrcpp::Any ConverterVisitor::visitExpression(CParser::ExpressionContext* ctx) {
	return visitChildren(ctx);
}

antlrcpp::Any ConverterVisitor::visitFile(CParser::FileContext* ctx) {
	return visitChildren(ctx);
}
