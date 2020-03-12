//
// Created by ward on 3/12/20.
//

#ifndef COMPILER_COMPILATIONERROR_H
#define COMPILER_COMPILATIONERROR_H

#include <stdexcept>

class CompilationError : std::logic_error {
public:
	CompilationError(const std::string& arg)
			:logic_error(arg) { }
};

#endif //COMPILER_COMPILATIONERROR_H
