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
    printf|
    IDENTIFIER '++'|
    IDENTIFIER '--';

prefixExpr:
    postfixExpr|
    '++' IDENTIFIER|
    '--' IDENTIFIER|
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
    IDENTIFIER '=' assignExpr;

specifier:
    'char'|
    'int'|
    'float';

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
    typeName IDENTIFIER ('=' initizalizer)?;

printf:
    'printf' '(' expr ')';

expr:
    assignExpr;

scopeStatement:
    '{' (statement | declaration? ';')* '}';

ifStatement:
    'if' '(' expr ')' statement ('else' statement)?;

whileStatement:
    'while' '(' expr ')' statement|
    'do' statement 'while' '(' expr ')' ';';

forStatement:
    'for' '(' (declaration | expr)? ';' expr? ';' expr? ')' statement;

exprStatement:
    (expr | 'break' | 'continue')? ';';

statement:
    exprStatement|
    scopeStatement|
    ifStatement |
    whileStatement | forStatement;

file:
    (declaration? ';' | scopeStatement)* EOF;




