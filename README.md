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
 - tests
 	 - benchmarkTests -	*provided tests*
 	 - customTests -	*tests written by ourselves about the required functionality*
 	 - optionalTests -	*tests about the opional functionality*
 	 - extraTests -	*tests about extra functionality (that was neither required or optional)*

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
 - Dereference and address-of operators in combination with multi-dimensional arrays and pointers
 - Literal strings as global arrays, assignable to char pointers
