//void f(int);
//
//int main()
//{
//	int a = 5;
//	printf("%d", a);
//	f(a);
//}
//
//#include <stdio.h>
//
//void f(int a)
//{
//	printf("%d", a);
//}

//////////////////////////////////////////////////////////

//#include <stdio.h>
//int main(){
//	int array[10][5];   //verkeerde volgorde
//	for (int i = 0; i<10; ++i)
//	{
//		for (int j = 0; j<5; ++j)
//		{
//			array[i][j] = i*10+j;
//			printf("%d", array[i][j]);
//		}
//	}
//}

//////////////////////////////////////////////////////////

//int main(){
//	const int a = 10;   //variabele wordt niet verwijderd uit ast in folding
//	int b = 5 + a;
//	int c= b + a;
//}

//////////////////////////////////////////////////////////

//int printf(int a){
//	return a+5;
//}
//
//int main(){
//	int x = printf(5);     //printf wordt gezien als variabele
//}

//////////////////////////////////////////////////////////

//int printf(int a){
//	return a+5;
//}
//
//int main(){
//	int x;
//	x = printf(5);
//	return x;
//}
//
//#include <stdio.h>    //meerdere defenitions

//////////////////////////////////////////////////////////

//#include <stdio.h>
//int main(){
//	int x[10];
//	int* a = x;
//
//	*x = 5;     //cannot dereference non-pointer type
//	printf("%d", x[0]);
//}

//////////////////////////////////////////////////////////

//int main()
//{
//	int x[10];
//	int* a = x;

//	*a = 5;
//	*x = 5;         //TODO
//	a[1] = 5;
//	x[1] = 5;
//
//	5+*a;
//	5+*x;           //TODO
//	5+a[1];
//	5+x[1];
//
//	int y[10][5];
//	int** b = y;
//	y[1][1] = 5;
//	(*y)[1] = 5;    //TODO
//	*(y[1]) = 5;    //TODO
//	**y = 5;        //TODO
//	b[1][1] = 5;
//	(*b)[1] = 5;
//	*(b[1]) = 5;
//	**b = 5;
//
//	5+y[1][1];
//	5+(*y)[1];      //TODO
//	5+*(y[1]);      //TODO
//	5+**y;          //TODO
//	5+b[1][1];
//	5+(*b)[1];
//	5+*(b[1]);
//	5+**b;
//}

//#include <stdio.h>
//int main()
//{
//	int x[10];
//	int* b = &x[1];
//	int* a = &x[0];
//	printf("%p, %p", a, b);
//}

int main()
{
	int z[5];
	int* a = (z)[5];
}
