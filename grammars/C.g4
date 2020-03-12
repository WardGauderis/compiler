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
    [0-9]+;

FLOAT:
    ([0-9]* '.' [0-9]+ | [0-9]+ '.') ([eE] [+-]? [0-9]+)? [fF]?|
    [0-9]+ ([eE] [+-]? [0-9]+) [fF]?;

IDENTIFIER:
    [a-zA-Z_] [a-zA-Z_0-9]*;

WS:[ \t\n\r]+->skip;

basicExpr:
    '(' orExpr ')'|
    INT;

unaryExpr:
    basicExpr|
    '+' unaryExpr|
    '-' unaryExpr|
    '!' unaryExpr;

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

//TODO: enkel =? bv. *=
assignExpr:
    orExpr|
    IDENTIFIER '=' assignExpr;

expr:
    assignExpr;

//TODO: opsplitsen? int weglaten?
typeName:
    QUALIFIER* SPECIFIER (QUALIFIER | '*')*|
    QUALIFIER (QUALIFIER | '*')*;

//TODO: comma separated? int x, y = 5
//TODO: enkel = initialization? x = {5}
initizalizer:
    expr
    ;

declaration:
    typeName IDENTIFIER ('=' initizalizer)? ';';

file:
    declaration EOF;








