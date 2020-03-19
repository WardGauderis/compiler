// this is a test file for checking: 'Assignment to an rvalue.'

int x = 1; // no error
4 = 6; // error
x + x = 5; // error
++x = 6; // error
x++ = 7; // error
5++; // error
++3; // error
(x = 2) = 3; // error
