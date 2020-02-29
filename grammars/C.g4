grammar C;

INT:[0-9]+;

WS:[ \t\n\r]+->skip;

basicExpression:
    '(' orExpression ')'|
    INT;

unaryExpression:
    basicExpression|
    '+' unaryExpression|
    '-' unaryExpression|
    '!' unaryExpression;

multiplicativeExpression:
    unaryExpression|
    multiplicativeExpression '*' unaryExpression|
    multiplicativeExpression '/' unaryExpression|
    multiplicativeExpression '%' unaryExpression;

additiveExpression:
    multiplicativeExpression|
    additiveExpression '+' multiplicativeExpression|
    additiveExpression '-' multiplicativeExpression;

relationalExpression:
    additiveExpression|
    relationalExpression '<' additiveExpression|
    relationalExpression '<=' additiveExpression|
    relationalExpression '>' additiveExpression|
    relationalExpression '>=' additiveExpression;

equalityExpression:
    relationalExpression|
    equalityExpression '==' relationalExpression|
    equalityExpression '!=' relationalExpression;

andExpression:
    equalityExpression|
    andExpression '&&' equalityExpression;

orExpression:
    andExpression|
    orExpression '||' andExpression;

expression:
    orExpression ';';

file:
    expression* EOF;








