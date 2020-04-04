grammar C;

QUALIFIER:
    'const';

CHAR:
    '\'' (~['\\\n\r] | '\\' .)+ '\'';

INT:
    [1-9] [0-9]*|
    '0' [0-7]*|             // 0 -> octaal
    '0' [xX] [0-9a-fA-F]+|
    '0' [bB] [01]+;

FLOAT:
    ([0-9]* '.' [0-9]+ | [0-9]+ '.') ([eE] [+-]? [0-9]+)? [fF]?|
    [0-9]+ ([eE] [+-]? [0-9]+) [fF]?;

STRING:
    ('"' (~["\\\n\r] | '\\' .)* '"');

IDENTIFIER:
    [a-zA-Z_] [a-zA-Z_0-9]*;

INCLUDESTDIO:
    '#' [ \t]* 'include' [ \t]* ('<stdio.h>' | '"stdio.h"');

LINECOMMENT:
    '//' ~[\n\r]* -> skip;

MULTILINECOMMENT:
    '/*' .*? '*/' -> skip;

WS:
    [ \t\n\r]+ -> skip;

literal:
    CHAR|
    INT|
    FLOAT|
    STRING+;

basicExpr:
    '(' expr ')'|
    IDENTIFIER|
    literal;

argumentList:
    expr (',' argumentList)?;

postfixExpr:
    basicExpr|
    IDENTIFIER '(' argumentList? ')'|
    postfixExpr '[' expr ']'|
    postfixExpr '++'|
    postfixExpr '--';

prefixExpr:
    postfixExpr|
    '++' prefixExpr|
    '--' prefixExpr|
    '*' prefixExpr|
    '&' prefixExpr|
    '+' prefixExpr|
    '-' prefixExpr|
    '!' prefixExpr|
    '(' typeName ')' prefixExpr;

multiplicativeExpr:
    prefixExpr|
    multiplicativeExpr '*' prefixExpr|
    multiplicativeExpr '/' prefixExpr|
    multiplicativeExpr '%' prefixExpr;

additiveExpr:
    multiplicativeExpr|
    additiveExpr '+' multiplicativeExpr|
    additiveExpr '-' multiplicativeExpr;

relationalExpr:
    additiveExpr|
    relationalExpr '<' additiveExpr|
    relationalExpr '<=' additiveExpr|
    relationalExpr '>' additiveExpr|
    relationalExpr '>=' additiveExpr;

equalityExpr:
    relationalExpr|
    equalityExpr '==' relationalExpr|
    equalityExpr '!=' relationalExpr;

andExpr:
    equalityExpr|
    andExpr '&&' equalityExpr;

orExpr:
    andExpr|
    orExpr '||' andExpr;

assignExpr:
    orExpr|
    prefixExpr '=' assignExpr;

specifier:
    'char'|
    'int'|
    'float'|
    'void';

qualifier:
    QUALIFIER+;

typeName:
     basicType pointerType?;

basicType:
    qualifier? specifier qualifier?;

pointerType:
    '*' qualifier? pointerType?;

declarationArray:
    '[' expr ']' declarationArray | ;

parameterArray:
    '[' expr? ']' parameterArray | ;

variableDeclaration:
    typeName IDENTIFIER ('=' assignExpr)? |
    typeName IDENTIFIER declarationArray;

declarationParameterList:
    typeName IDENTIFIER? parameterArray (',' declarationParameterList)?;

parameterList:
    typeName IDENTIFIER parameterArray (',' parameterList)?;

functionDeclaration:
    typeName IDENTIFIER '(' declarationParameterList? ')';

functionDefinition:
    typeName IDENTIFIER '(' parameterList? ')' scopeStatement;

declaration:
    variableDeclaration | functionDeclaration ';';

expr:
    assignExpr;

scopeStatement:
    '{' (statement | declaration)* '}';

ifStatement:
    'if' '(' expr ')' statement ('else' statement)?;

whileStatement:
    'while' '(' expr ')' statement |
    'do' statement 'while' '(' expr ')' ';';

forStatement:
    'for' '(' (variableDeclaration | expr)? ';'  expr? ';'  expr? ')' statement;

exprStatement:
    expr? ';';

controlStatement:
    ('break' | 'continue') ';' | 'return' exprStatement;

statement:
    exprStatement |
    controlStatement |
    scopeStatement |
    ifStatement |
    whileStatement |
    forStatement;

file:
    (declaration | functionDefinition | ';' | INCLUDESTDIO)* EOF;




