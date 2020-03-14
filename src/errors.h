//============================================================================
// @author      : Thomas Dooms & Ward Gauderis
// @date        : 3/10/20
// @copyright   : BA2 Informatica - Thomas Dooms & Ward Gauderis - University of Antwerp
//============================================================================

#pragma once

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
