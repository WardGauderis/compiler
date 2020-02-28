
// Generated from /home/thomas/CLionProjects/compiler/grammars/JSON.g4 by ANTLR 4.8


#include "JSONListener.h"
#include "JSONVisitor.h"

#include "JSONParser.h"


using namespace antlrcpp;
using namespace antlr4;

JSONParser::JSONParser(TokenStream *input) : Parser(input) {
  _interpreter = new atn::ParserATNSimulator(this, _atn, _decisionToDFA, _sharedContextCache);
}

JSONParser::~JSONParser() {
  delete _interpreter;
}

std::string JSONParser::getGrammarFileName() const {
  return "JSON.g4";
}

const std::vector<std::string>& JSONParser::getRuleNames() const {
  return _ruleNames;
}

dfa::Vocabulary& JSONParser::getVocabulary() const {
  return _vocabulary;
}


//----------------- JsonContext ------------------------------------------------------------------

JSONParser::JsonContext::JsonContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

JSONParser::ObjectContext* JSONParser::JsonContext::object() {
  return getRuleContext<JSONParser::ObjectContext>(0);
}

JSONParser::ArrayContext* JSONParser::JsonContext::array() {
  return getRuleContext<JSONParser::ArrayContext>(0);
}


size_t JSONParser::JsonContext::getRuleIndex() const {
  return JSONParser::RuleJson;
}

void JSONParser::JsonContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<JSONListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterJson(this);
}

void JSONParser::JsonContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<JSONListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitJson(this);
}


antlrcpp::Any JSONParser::JsonContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<JSONVisitor*>(visitor))
    return parserVisitor->visitJson(this);
  else
    return visitor->visitChildren(this);
}

JSONParser::JsonContext* JSONParser::json() {
  JsonContext *_localctx = _tracker.createInstance<JsonContext>(_ctx, getState());
  enterRule(_localctx, 0, JSONParser::RuleJson);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    setState(12);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case JSONParser::T__0: {
        enterOuterAlt(_localctx, 1);
        setState(10);
        object();
        break;
      }

      case JSONParser::T__4: {
        enterOuterAlt(_localctx, 2);
        setState(11);
        array();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ObjectContext ------------------------------------------------------------------

JSONParser::ObjectContext::ObjectContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<JSONParser::PairContext *> JSONParser::ObjectContext::pair() {
  return getRuleContexts<JSONParser::PairContext>();
}

JSONParser::PairContext* JSONParser::ObjectContext::pair(size_t i) {
  return getRuleContext<JSONParser::PairContext>(i);
}


size_t JSONParser::ObjectContext::getRuleIndex() const {
  return JSONParser::RuleObject;
}

void JSONParser::ObjectContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<JSONListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterObject(this);
}

void JSONParser::ObjectContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<JSONListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitObject(this);
}


antlrcpp::Any JSONParser::ObjectContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<JSONVisitor*>(visitor))
    return parserVisitor->visitObject(this);
  else
    return visitor->visitChildren(this);
}

JSONParser::ObjectContext* JSONParser::object() {
  ObjectContext *_localctx = _tracker.createInstance<ObjectContext>(_ctx, getState());
  enterRule(_localctx, 2, JSONParser::RuleObject);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    setState(27);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 2, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(14);
      match(JSONParser::T__0);
      setState(15);
      pair();
      setState(20);
      _errHandler->sync(this);
      _la = _input->LA(1);
      while (_la == JSONParser::T__1) {
        setState(16);
        match(JSONParser::T__1);
        setState(17);
        pair();
        setState(22);
        _errHandler->sync(this);
        _la = _input->LA(1);
      }
      setState(23);
      match(JSONParser::T__2);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(25);
      match(JSONParser::T__0);
      setState(26);
      match(JSONParser::T__2);
      break;
    }

    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- PairContext ------------------------------------------------------------------

JSONParser::PairContext::PairContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* JSONParser::PairContext::STRING() {
  return getToken(JSONParser::STRING, 0);
}

JSONParser::ValueContext* JSONParser::PairContext::value() {
  return getRuleContext<JSONParser::ValueContext>(0);
}


size_t JSONParser::PairContext::getRuleIndex() const {
  return JSONParser::RulePair;
}

void JSONParser::PairContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<JSONListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterPair(this);
}

void JSONParser::PairContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<JSONListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitPair(this);
}


antlrcpp::Any JSONParser::PairContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<JSONVisitor*>(visitor))
    return parserVisitor->visitPair(this);
  else
    return visitor->visitChildren(this);
}

JSONParser::PairContext* JSONParser::pair() {
  PairContext *_localctx = _tracker.createInstance<PairContext>(_ctx, getState());
  enterRule(_localctx, 4, JSONParser::RulePair);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(29);
    match(JSONParser::STRING);
    setState(30);
    match(JSONParser::T__3);
    setState(31);
    value();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ArrayContext ------------------------------------------------------------------

JSONParser::ArrayContext::ArrayContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<JSONParser::ValueContext *> JSONParser::ArrayContext::value() {
  return getRuleContexts<JSONParser::ValueContext>();
}

JSONParser::ValueContext* JSONParser::ArrayContext::value(size_t i) {
  return getRuleContext<JSONParser::ValueContext>(i);
}


size_t JSONParser::ArrayContext::getRuleIndex() const {
  return JSONParser::RuleArray;
}

void JSONParser::ArrayContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<JSONListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterArray(this);
}

void JSONParser::ArrayContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<JSONListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitArray(this);
}


antlrcpp::Any JSONParser::ArrayContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<JSONVisitor*>(visitor))
    return parserVisitor->visitArray(this);
  else
    return visitor->visitChildren(this);
}

JSONParser::ArrayContext* JSONParser::array() {
  ArrayContext *_localctx = _tracker.createInstance<ArrayContext>(_ctx, getState());
  enterRule(_localctx, 6, JSONParser::RuleArray);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    setState(46);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 4, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(33);
      match(JSONParser::T__4);
      setState(34);
      value();
      setState(39);
      _errHandler->sync(this);
      _la = _input->LA(1);
      while (_la == JSONParser::T__1) {
        setState(35);
        match(JSONParser::T__1);
        setState(36);
        value();
        setState(41);
        _errHandler->sync(this);
        _la = _input->LA(1);
      }
      setState(42);
      match(JSONParser::T__5);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(44);
      match(JSONParser::T__4);
      setState(45);
      match(JSONParser::T__5);
      break;
    }

    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ValueContext ------------------------------------------------------------------

JSONParser::ValueContext::ValueContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* JSONParser::ValueContext::STRING() {
  return getToken(JSONParser::STRING, 0);
}

tree::TerminalNode* JSONParser::ValueContext::NUMBER() {
  return getToken(JSONParser::NUMBER, 0);
}

JSONParser::ObjectContext* JSONParser::ValueContext::object() {
  return getRuleContext<JSONParser::ObjectContext>(0);
}

JSONParser::ArrayContext* JSONParser::ValueContext::array() {
  return getRuleContext<JSONParser::ArrayContext>(0);
}


size_t JSONParser::ValueContext::getRuleIndex() const {
  return JSONParser::RuleValue;
}

void JSONParser::ValueContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<JSONListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterValue(this);
}

void JSONParser::ValueContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<JSONListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitValue(this);
}


antlrcpp::Any JSONParser::ValueContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<JSONVisitor*>(visitor))
    return parserVisitor->visitValue(this);
  else
    return visitor->visitChildren(this);
}

JSONParser::ValueContext* JSONParser::value() {
  ValueContext *_localctx = _tracker.createInstance<ValueContext>(_ctx, getState());
  enterRule(_localctx, 8, JSONParser::RuleValue);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    setState(55);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case JSONParser::STRING: {
        enterOuterAlt(_localctx, 1);
        setState(48);
        match(JSONParser::STRING);
        break;
      }

      case JSONParser::NUMBER: {
        enterOuterAlt(_localctx, 2);
        setState(49);
        match(JSONParser::NUMBER);
        break;
      }

      case JSONParser::T__0: {
        enterOuterAlt(_localctx, 3);
        setState(50);
        object();
        break;
      }

      case JSONParser::T__4: {
        enterOuterAlt(_localctx, 4);
        setState(51);
        array();
        break;
      }

      case JSONParser::T__6: {
        enterOuterAlt(_localctx, 5);
        setState(52);
        match(JSONParser::T__6);
        break;
      }

      case JSONParser::T__7: {
        enterOuterAlt(_localctx, 6);
        setState(53);
        match(JSONParser::T__7);
        break;
      }

      case JSONParser::T__8: {
        enterOuterAlt(_localctx, 7);
        setState(54);
        match(JSONParser::T__8);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

// Static vars and initialization.
std::vector<dfa::DFA> JSONParser::_decisionToDFA;
atn::PredictionContextCache JSONParser::_sharedContextCache;

// We own the ATN which in turn owns the ATN states.
atn::ATN JSONParser::_atn;
std::vector<uint16_t> JSONParser::_serializedATN;

std::vector<std::string> JSONParser::_ruleNames = {
  "json", "object", "pair", "array", "value"
};

std::vector<std::string> JSONParser::_literalNames = {
  "", "'{'", "','", "'}'", "':'", "'['", "']'", "'true'", "'false'", "'null'"
};

std::vector<std::string> JSONParser::_symbolicNames = {
  "", "", "", "", "", "", "", "", "", "", "STRING", "NUMBER", "WS"
};

dfa::Vocabulary JSONParser::_vocabulary(_literalNames, _symbolicNames);

std::vector<std::string> JSONParser::_tokenNames;

JSONParser::Initializer::Initializer() {
	for (size_t i = 0; i < _symbolicNames.size(); ++i) {
		std::string name = _vocabulary.getLiteralName(i);
		if (name.empty()) {
			name = _vocabulary.getSymbolicName(i);
		}

		if (name.empty()) {
			_tokenNames.push_back("<INVALID>");
		} else {
      _tokenNames.push_back(name);
    }
	}

  _serializedATN = {
    0x3, 0x608b, 0xa72a, 0x8133, 0xb9ed, 0x417c, 0x3be7, 0x7786, 0x5964, 
    0x3, 0xe, 0x3c, 0x4, 0x2, 0x9, 0x2, 0x4, 0x3, 0x9, 0x3, 0x4, 0x4, 0x9, 
    0x4, 0x4, 0x5, 0x9, 0x5, 0x4, 0x6, 0x9, 0x6, 0x3, 0x2, 0x3, 0x2, 0x5, 
    0x2, 0xf, 0xa, 0x2, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x7, 0x3, 
    0x15, 0xa, 0x3, 0xc, 0x3, 0xe, 0x3, 0x18, 0xb, 0x3, 0x3, 0x3, 0x3, 0x3, 
    0x3, 0x3, 0x3, 0x3, 0x5, 0x3, 0x1e, 0xa, 0x3, 0x3, 0x4, 0x3, 0x4, 0x3, 
    0x4, 0x3, 0x4, 0x3, 0x5, 0x3, 0x5, 0x3, 0x5, 0x3, 0x5, 0x7, 0x5, 0x28, 
    0xa, 0x5, 0xc, 0x5, 0xe, 0x5, 0x2b, 0xb, 0x5, 0x3, 0x5, 0x3, 0x5, 0x3, 
    0x5, 0x3, 0x5, 0x5, 0x5, 0x31, 0xa, 0x5, 0x3, 0x6, 0x3, 0x6, 0x3, 0x6, 
    0x3, 0x6, 0x3, 0x6, 0x3, 0x6, 0x3, 0x6, 0x5, 0x6, 0x3a, 0xa, 0x6, 0x3, 
    0x6, 0x2, 0x2, 0x7, 0x2, 0x4, 0x6, 0x8, 0xa, 0x2, 0x2, 0x2, 0x41, 0x2, 
    0xe, 0x3, 0x2, 0x2, 0x2, 0x4, 0x1d, 0x3, 0x2, 0x2, 0x2, 0x6, 0x1f, 0x3, 
    0x2, 0x2, 0x2, 0x8, 0x30, 0x3, 0x2, 0x2, 0x2, 0xa, 0x39, 0x3, 0x2, 0x2, 
    0x2, 0xc, 0xf, 0x5, 0x4, 0x3, 0x2, 0xd, 0xf, 0x5, 0x8, 0x5, 0x2, 0xe, 
    0xc, 0x3, 0x2, 0x2, 0x2, 0xe, 0xd, 0x3, 0x2, 0x2, 0x2, 0xf, 0x3, 0x3, 
    0x2, 0x2, 0x2, 0x10, 0x11, 0x7, 0x3, 0x2, 0x2, 0x11, 0x16, 0x5, 0x6, 
    0x4, 0x2, 0x12, 0x13, 0x7, 0x4, 0x2, 0x2, 0x13, 0x15, 0x5, 0x6, 0x4, 
    0x2, 0x14, 0x12, 0x3, 0x2, 0x2, 0x2, 0x15, 0x18, 0x3, 0x2, 0x2, 0x2, 
    0x16, 0x14, 0x3, 0x2, 0x2, 0x2, 0x16, 0x17, 0x3, 0x2, 0x2, 0x2, 0x17, 
    0x19, 0x3, 0x2, 0x2, 0x2, 0x18, 0x16, 0x3, 0x2, 0x2, 0x2, 0x19, 0x1a, 
    0x7, 0x5, 0x2, 0x2, 0x1a, 0x1e, 0x3, 0x2, 0x2, 0x2, 0x1b, 0x1c, 0x7, 
    0x3, 0x2, 0x2, 0x1c, 0x1e, 0x7, 0x5, 0x2, 0x2, 0x1d, 0x10, 0x3, 0x2, 
    0x2, 0x2, 0x1d, 0x1b, 0x3, 0x2, 0x2, 0x2, 0x1e, 0x5, 0x3, 0x2, 0x2, 
    0x2, 0x1f, 0x20, 0x7, 0xc, 0x2, 0x2, 0x20, 0x21, 0x7, 0x6, 0x2, 0x2, 
    0x21, 0x22, 0x5, 0xa, 0x6, 0x2, 0x22, 0x7, 0x3, 0x2, 0x2, 0x2, 0x23, 
    0x24, 0x7, 0x7, 0x2, 0x2, 0x24, 0x29, 0x5, 0xa, 0x6, 0x2, 0x25, 0x26, 
    0x7, 0x4, 0x2, 0x2, 0x26, 0x28, 0x5, 0xa, 0x6, 0x2, 0x27, 0x25, 0x3, 
    0x2, 0x2, 0x2, 0x28, 0x2b, 0x3, 0x2, 0x2, 0x2, 0x29, 0x27, 0x3, 0x2, 
    0x2, 0x2, 0x29, 0x2a, 0x3, 0x2, 0x2, 0x2, 0x2a, 0x2c, 0x3, 0x2, 0x2, 
    0x2, 0x2b, 0x29, 0x3, 0x2, 0x2, 0x2, 0x2c, 0x2d, 0x7, 0x8, 0x2, 0x2, 
    0x2d, 0x31, 0x3, 0x2, 0x2, 0x2, 0x2e, 0x2f, 0x7, 0x7, 0x2, 0x2, 0x2f, 
    0x31, 0x7, 0x8, 0x2, 0x2, 0x30, 0x23, 0x3, 0x2, 0x2, 0x2, 0x30, 0x2e, 
    0x3, 0x2, 0x2, 0x2, 0x31, 0x9, 0x3, 0x2, 0x2, 0x2, 0x32, 0x3a, 0x7, 
    0xc, 0x2, 0x2, 0x33, 0x3a, 0x7, 0xd, 0x2, 0x2, 0x34, 0x3a, 0x5, 0x4, 
    0x3, 0x2, 0x35, 0x3a, 0x5, 0x8, 0x5, 0x2, 0x36, 0x3a, 0x7, 0x9, 0x2, 
    0x2, 0x37, 0x3a, 0x7, 0xa, 0x2, 0x2, 0x38, 0x3a, 0x7, 0xb, 0x2, 0x2, 
    0x39, 0x32, 0x3, 0x2, 0x2, 0x2, 0x39, 0x33, 0x3, 0x2, 0x2, 0x2, 0x39, 
    0x34, 0x3, 0x2, 0x2, 0x2, 0x39, 0x35, 0x3, 0x2, 0x2, 0x2, 0x39, 0x36, 
    0x3, 0x2, 0x2, 0x2, 0x39, 0x37, 0x3, 0x2, 0x2, 0x2, 0x39, 0x38, 0x3, 
    0x2, 0x2, 0x2, 0x3a, 0xb, 0x3, 0x2, 0x2, 0x2, 0x8, 0xe, 0x16, 0x1d, 
    0x29, 0x30, 0x39, 
  };

  atn::ATNDeserializer deserializer;
  _atn = deserializer.deserialize(_serializedATN);

  size_t count = _atn.getNumberOfDecisions();
  _decisionToDFA.reserve(count);
  for (size_t i = 0; i < count; i++) { 
    _decisionToDFA.emplace_back(_atn.getDecisionState(i), i);
  }
}

JSONParser::Initializer JSONParser::_init;
