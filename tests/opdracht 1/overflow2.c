#include <stdio.h>

int main()
{
    int         a = 7;
    const float b = 5.0;

    printf("%i", 5);
    printf("%i", a);
    printf("%i", a++);
    printf("%i", ++a);
    printf("%f", 0. / 0.);
    printf("%f", 5. / 0.);
    printf("%f", 50000 * 50 * 5000000000000000000000000000);
    printf("%f", 5e+55555555555F);
}