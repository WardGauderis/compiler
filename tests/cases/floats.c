#include <stdio.h>

float a = 5. / 2.;
float b = 5. / 2;
float c = 5 / 2.;
float d = 6 - 1;
float e = 5. / 0.;
float f = 1 * 1.5;
float g = (int)5.7;

int main()
{
    printf("%f", a);
    printf("%f", b);
    printf("%f", c);
    printf("%f", d);
    printf("%f", e);
    printf("%f", f);
    printf("%f", g);
}