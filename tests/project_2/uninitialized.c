// this is a test file for checking: 'Use of an undefined or uninitialized variable.'

#include <stdio.h>

int main()
{
    int a;
    printf("%i", a);
    a = a + 5;
    printf("%i", a);
    a + 5;
}