# C Compiler
### Compilers
#### Thomas Dooms - Ward Gauderis - UAntwerpen bachelor 2 Informatica

#### Dependencies:
 - antlr4-runtime
 - LLVM
 - boost_program_options
 - Graphviz Dot

#### Compilation:
 - ./build.sh

#### Usage:
 - ./compiler \<files> 
 - ./compiler -h

#### Tests:
File structure:
 - tests'
 	 - benchmark - *provided tests*
 	 - project_<1-6> - *custom tests for each project*
 	    - files prefixed with 'opt' are for optional features
 	    - files prefixed with 'extra' are for extra features (neither required nor optional)
 	 - opt-extra - *tests for extra and optional features*

#### Execution of tests:
 - ./test.sh

#### Optional features:
 - Binary operator: %
 - Comparison operators:  >=, <=, !=
 - Logical operator: &&, ||, !
 - Constant folding
 - Unary operators: ++, -- (prefix and postfix)
 - Conversions: conversion operator, warnings
 - Constant propagation
 - Check return statements (not in all paths of the function)
 - Unused variable elimination
 - Constexpr conditional elimination
 - Multi-dimensional arrays

#### Extra features:
 - Hexadecimal, octal, binary integer literals
 - CST visualisation
 - Float literals in scientific notation
 - Do while loops
 - Dereference and address-of operators in combination with multi-dimensional arrays and pointers
 - Literal strings as global arrays, assignable to char pointers
