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
//
//	*a = 5;
//	*x = 5;
//	a[1] = 5;
//	x[1] = 5;
//
//	5+*a;
//	5+*x;
//	5+a[1];
//	5+x[1];
//
//	int y[10][5];
//	int** b = y;
//	y[1][1] = 5;
//	(*y)[1] = 5;
//	*(y[1]) = 5;
//	**y = 5;
//	b[1][1] = 5;
//	(*b)[1] = 5;
//	*(b[1]) = 5;
//	**b = 5;
//
//	5+y[1][1];
//	5+(*y)[1];
//	5+*(y[1]);
//	5+**y;
//	5+b[1][1];
//	5+(*b)[1];
//	5+*(b[1]);
//	5+**b;
//}

//#include <stdio.h>
//int main()
//{
//	int* a[10][5];
//	int ** b = &*a[3];
//	int * c = (&((*a)[3]))[2];
//	int d = &((&c[3])[3]);
//	int e = *&*(5+a)[2];      //TODO
//	*(&(*a[10])[5]) = &b[3];
//}

int main(){
	int x[10];
	int y[5];
	x[2] = y[2];
}
