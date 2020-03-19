// this folder is for checking: 'Operations or assignments of incompatible types.'
// this file specifically tests binary pointer operations

int* a; // warning unassigned
a + 5; // no error
a + 5.5; // error
a && 1; // no error
a * 5; // error
a / 2; // error
a % 2; // error
a - 6; // no error

