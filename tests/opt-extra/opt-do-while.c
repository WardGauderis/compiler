#include <stdio.h>

int main(){
    int a = 0;
    while (++a < 7) {
		printf("%d ", a);
    }
    printf("\n");
    a = 0;
    do
    {
        printf("%d ", a);
    } while(++a < 7);
}
