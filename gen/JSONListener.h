
// Generated from /home/thomas/CLionProjects/compiler/grammars/JSON.g4 by ANTLR 4.8

#pragma once


#include "antlr4-runtime.h"
#include "JSONParser.h"


/**
 * This interface defines an abstract listener for a parse tree produced by JSONParser.
 */
class  JSONListener : public antlr4::tree::ParseTreeListener {
public:

  virtual void enterJson(JSONParser::JsonContext *ctx) = 0;
  virtual void exitJson(JSONParser::JsonContext *ctx) = 0;

  virtual void enterObject(JSONParser::ObjectContext *ctx) = 0;
  virtual void exitObject(JSONParser::ObjectContext *ctx) = 0;

  virtual void enterPair(JSONParser::PairContext *ctx) = 0;
  virtual void exitPair(JSONParser::PairContext *ctx) = 0;

  virtual void enterArray(JSONParser::ArrayContext *ctx) = 0;
  virtual void exitArray(JSONParser::ArrayContext *ctx) = 0;

  virtual void enterValue(JSONParser::ValueContext *ctx) = 0;
  virtual void exitValue(JSONParser::ValueContext *ctx) = 0;


};

