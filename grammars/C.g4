grammar C;

INT:[0-9]+;

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

expr:
    orExpr ';';

file:
    expr* EOF;








