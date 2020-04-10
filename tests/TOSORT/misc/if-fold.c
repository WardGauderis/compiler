#include <stdio.h>

int main()
{
    const int a = 5;
    if(a)
    {
        printf("%i", 6);
    }

    if(4)
    {
        printf("%i", a);
    }
    else
    {
        printf("%i", 777);
    }

    if(0)
    {
        printf("%i", 555555); // this will disappear
    }
}