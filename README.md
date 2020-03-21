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

#### Execution of tests:
 - ./test.sh

#### Optional features:
 - Binary operator: %
 - Comparison operators:  >=, <=, !=
 - Logical operator: &&, ||, ! (not yet supported in LLVM because of short-circuit evaluation)
 - Constant folding
 - Unary operators: ++, -- (prefix and postfix)
 - Conversions: conversion operator, warnings
 - Constant propagation

#### Extra features:
 - Hexadecimal, octal, binary integer literals
 - CST visualisation
 - Float literals in scientific notation
 - Printf accepts r-values that are a result of an expression
