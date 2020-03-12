//
// Created by ward on 3/12/20.
//

#ifndef COMPILER_ERRORS_H
#define COMPILER_ERRORS_H

#include <stdexcept>

class CompilationError : public std::logic_error {
	using std::logic_error::logic_error;
};

class SyntaxError : public CompilationError {
	using CompilationError::CompilationError;
};

class SemanticError : public CompilationError {
	using CompilationError::CompilationError;
};

class WhoopsiePoopsieError : public std::runtime_error {
	using std::runtime_error::runtime_error;
};

#endif //COMPILER_ERRORS_H
