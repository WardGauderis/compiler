#include <stdio.h>

int main()
{
    char*  a = 1;
    float* b = a;

    a = 2 + b;

    b = a + 2;

    printf("%i", (int)a);
    printf("%i", (int)b);
}