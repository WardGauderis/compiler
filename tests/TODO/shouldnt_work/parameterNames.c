int f(int a, float b, char a){	// TODO verkeerde error en segfault
	return b;
}

#include <stdio.h>

int main(){
	printf("%d\n", f(5, 5.5, 'a'));
}
