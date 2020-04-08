#include <stdio.h>
int main(){
	int x[10];
	int* a = x;

	*x = 5;
	printf("%d", x[0]);
}
