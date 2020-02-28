
// Generated from /home/thomas/CLionProjects/compiler/grammars/JSON.g4 by ANTLR 4.8

#pragma once


#include "antlr4-runtime.h"
#include "JSONListener.h"


/**
 * This class provides an empty implementation of JSONListener,
 * which can be extended to create a listener which only needs to handle a subset
 * of the available methods.
 */
class  JSONBaseListener : public JSONListener {
public:

  virtual void enterJson(JSONParser::JsonContext * /*ctx*/) override { }
  virtual void exitJson(JSONParser::JsonContext * /*ctx*/) override { }

  virtual void enterObject(JSONParser::ObjectContext * /*ctx*/) override { }
  virtual void exitObject(JSONParser::ObjectContext * /*ctx*/) override { }

  virtual void enterPair(JSONParser::PairContext * /*ctx*/) override { }
  virtual void exitPair(JSONParser::PairContext * /*ctx*/) override { }

  virtual void enterArray(JSONParser::ArrayContext * /*ctx*/) override { }
  virtual void exitArray(JSONParser::ArrayContext * /*ctx*/) override { }

  virtual void enterValue(JSONParser::ValueContext * /*ctx*/) override { }
  virtual void exitValue(JSONParser::ValueContext * /*ctx*/) override { }


  virtual void enterEveryRule(antlr4::ParserRuleContext * /*ctx*/) override { }
  virtual void exitEveryRule(antlr4::ParserRuleContext * /*ctx*/) override { }
  virtual void visitTerminal(antlr4::tree::TerminalNode * /*node*/) override { }
  virtual void visitErrorNode(antlr4::tree::ErrorNode * /*node*/) override { }

};

