// constant folding and constant propagation

#include <stdio.h>

int main()
{
    const int a = 5;
    const int b = 6;
    int       c = a + b; // can fold

    int d = 5;
    int e = 6;
    int f = d + e; // cannot fold

    (4 + 6) / 2;
    (3 + 5) / 0; //! will not fold
    printf("%i", a);
    printf("%i", b);
    printf("%i", c);
    printf("%i", d);
    printf("%i", e);
    printf("%i", f);
}