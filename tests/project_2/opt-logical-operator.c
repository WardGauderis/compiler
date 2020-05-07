#include <stdio.h>

int f()
{
	printf("f wordt uitgevoerd.\n");
	return 1;
}

int g()
{
	printf("g wordt uitgevoerd.\n");
	return 0;
}

int main()
{
	f() && f();
	f() && g();
	g() && f();
	g() && g();
	f() || f();
	f() || g();
	g() || f();
	g() || g();
}
