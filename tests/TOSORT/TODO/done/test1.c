void f(int b[], int [5]);


int main()
{
	int*a;
	int*b;
	f(a, b);
	return 0;
}

#include <stdio.h>
void f(int* a, int* b){
	printf("hallo\n");
}
