#!/usr/bin/env sh
antlr4 -o gen -Dlanguage=Cpp grammars/C.g4
cmake .
make