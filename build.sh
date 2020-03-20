#!/usr/bin/env sh
antlr4 -o gen -Dlanguage=Cpp C.g4
cmake .
make