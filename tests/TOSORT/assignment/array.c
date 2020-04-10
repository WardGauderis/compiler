// this is a test for the assign arrays feature
#include <stdio.h>

int main()
{
    int a[10][5];
    int b[5];
    int** c = a;
    c[5] = b;

    for (int i = 0; i<5; ++i)
    {
        printf("%d\n", c[5][i]);
    }
}
