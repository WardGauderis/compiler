// this folder is for checking: 'Operations or assignments of incompatible types.'
// this file specifically tests unary pointer operations

int* x = 5; // warning unassigned
+x; // error
-x; // error
!x; //no error
++x; // no error
--x; // no error
x++; // no error
x--; // no error


