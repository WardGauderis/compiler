grammar C;

QUALIFIER:
    'const';

CHAR:
    '\'' (~['\\] | '\\' .)+ '\'';

INT:
    [1-9] [0-9]*|
    '0' [0-7]*|             // 0 -> octaal
    '0' [xX] [0-9a-fA-F]+|
    '0' [bB] [01]+;

FLOAT:
    ([0-9]* '.' [0-9]+ | [0-9]+ '.') ([eE] [+-]? [0-9]+)? [fF]?|
    [0-9]+ ([eE] [+-]? [0-9]+) [fF]?;

IDENTIFIER:
    [a-zA-Z_] [a-zA-Z_0-9]*;

LINECOMMENT:
    '//' ~[\n\r]* -> skip;

MULTILINECOMMENT:
    '/*' .*? '*/' -> skip;

WS:
    [ \t\n\r]+ -> skip;

literal:
    CHAR|
    INT|
    FLOAT;

printf:
    'printf' '(' expr ')';

basicExpr:
    '(' expr ')'|
    IDENTIFIER|
    literal;

postfixExpr:
    basicExpr|
    IDENTIFIER '(' argumentList? ')'|
    printf|
    postfixExpr '++'|
    postfixExpr '--';

prefixExpr:
    postfixExpr|
    '++' prefixExpr|
    '--' prefixExpr|
    '&' prefixExpr|
    '*' prefixExpr|
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

initizalizer:
    assignExpr;

declaration:
    typeName IDENTIFIER ('=' initizalizer)? ';';

expr:
    assignExpr;

scopeStatement:
    '{' (statement | declaration)* '}';

ifStatement:
    'if' '(' expr ')' statement ('else' statement)?;

whileStatement:
    'while' '(' expr ')' statement|
    'do' statement 'while' '(' expr ')' ';';

forStatement:
    'for' '(' (declaration | exprStatement)  expr? ';'  expr? ')' statement;

exprStatement:
    expr? ';';

controlStatement:
    ('break' | 'continue') ';' | 'return' exprStatement;

statement:
    exprStatement|
    controlStatement|
    scopeStatement|
    ifStatement |
    whileStatement |
    forStatement;

parameterList:
    typeName IDENTIFIER (',' parameterList)?;

argumentList:
    expr (',' argumentList)?;

functionDefinition:
    typeName IDENTIFIER '(' parameterList? ')' scopeStatement ';'?;

file:
    (declaration | functionDefinition | ';' )* EOF;




