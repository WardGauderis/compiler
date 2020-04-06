#include <stdio.h>

int main()
{
    int a = 1;
    int x = 5.;
    int y = (x = 7.) - (a = 0);
    printf("%i", x - y);
    int   t = 5;
    float f = 4.99;
    int   z = t < f;
    printf("%i", z);

    int q[10][5];
}