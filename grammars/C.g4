grammar C;

SPECIFIER:
    'char'|
    'int'|
    'float';

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

WS:[ \t\n\r]+->skip;

basicExpr:
    '(' orExpr ')'|
    INT;

postfixExpr:
    basicExpr|
    postfixExpr '++'|
    postfixExpr '--';

prefixExpr:
    basicExpr|
    '++' prefixExpr|
    '--' prefixExpr|
    '+' prefixExpr|
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

//TODO: enkel =? bv. *=
assignExpr:
    orExpr|
    IDENTIFIER '=' assignExpr;

expr:
    assignExpr;

typeName:
    QUALIFIER* SPECIFIER QUALIFIER* pointerType?;

pointerType:
    '*' QUALIFIER* pointerType?;

initizalizer:
    expr;

declaration:
    typeName IDENTIFIER ('=' initizalizer)? ';';

file:
    (declaration | assignExpr ';')* EOF;








