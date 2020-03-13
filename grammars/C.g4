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

LINECOMMENT:
    '//' ~[\n\r]*;

MULTILINECOMMENT:
    '/*' .*? '*/';

WS:
    [ \t\n\r]+ -> skip;

comment:
    LINECOMMENT | MULTILINECOMMENT;

literal:
    CHAR|
    INT|
    FLOAT;

basicExpr:
    '(' expr ')'|
    IDENTIFIER|
    literal;

postfixExpr:
    basicExpr|
    IDENTIFIER '++'|
    IDENTIFIER '--';

prefixExpr:
    postfixExpr|
    '++' IDENTIFIER|
    '--' IDENTIFIER;

unaryExpr:
    prefixExpr|
    '+' unaryExpr|
    '-' unaryExpr|
    '!' unaryExpr|
    '(' typeName ')' unaryExpr;

multiplicativeExpr:
    unaryExpr|
    multiplicativeExpr '*' unaryExpr|
    multiplicativeExpr '/' unaryExpr|
    multiplicativeExpr '%' unaryExpr;

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
    IDENTIFIER '=' assignExpr;

expr:
    assignExpr;

qualifier:
    QUALIFIER+;

typeName:
    qualifier? SPECIFIER qualifier? pointerType?;

pointerType:
    '*' qualifier? pointerType?;

initizalizer:
    expr;

declaration:
    typeName IDENTIFIER ('=' initizalizer)?;

printf:
    'printf' '(' (IDENTIFIER | literal) ')';

statement:
    (declaration | expr | printf) ';';

block:
    (statement | comment)* EOF;








