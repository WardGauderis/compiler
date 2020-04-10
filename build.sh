#!/usr/bin/env sh
antlr4 -o gen -Dlanguage=Cpp C.g4
mkdir build
pushd build
cmake ..
make
popd
mv build/compiler bin/compiler
