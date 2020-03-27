// this folder is for checking: 'Operations or assignments of incompatible types.'
// this file specifically tests binary pointer operations

int* a; // warning
float* b; // warning

{
a + 5; // no error
a + 5.5; // error
a && 1; // no error
a * 5; // error
a / 2; // error
a % 2; // error
a - 6; // no error
6 - a; // error

a <= 5.5; // error
5.5 > a; // error
a || 5.5; // no error

a != b; // no error
b == a; // no error
b < a; // no error
a > 5.5; // error
}
