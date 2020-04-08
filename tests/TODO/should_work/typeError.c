void f(int);

int main()    //TODO internal error
{
	int a = 5;
	f(a);
}

#include <stdio.h>

void f(int a)
{
	printf("%d", a);
}
