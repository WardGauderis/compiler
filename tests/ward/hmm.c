int f = 6;
#include <stdio.h>

int g(int* f){
    return (*f)++;
}

int main()
{
    printf(g(&f));
    printf(f);
}
