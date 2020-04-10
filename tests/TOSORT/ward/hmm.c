int f = 6;
#include <stdio.h>

int g(int* f){
    return (*f)++;
}

int main()
{
    printf("%i", g(&f));
    printf("%i", f);
}
