#include <stdio.h>

int g(int* f)
{
    return (*f)++;
}

int main()
{
    const int f = 10;
    printf("%i", g(&f));
    printf("%i", f);
}
