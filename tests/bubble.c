//#include <stdio.h>
//
//void bubble_sort (int *a, int n) {
//	int i, t, j = n, s = 1;
//	while (s) {
//		s = 0;
//		for (i = 1; i < j; i++) {
//			if (a[i] < a[i - 1]) {
//				t = a[i];
//				a[i] = a[i - 1];
//				a[i - 1] = t;
//				s = 1;
//			}
//		}
//		j--;
//	}
//}
//
//int main () {
//	int a[10];
//	a[0] = 4;
//	a[1] = 65;
//	a[2] = 2;
//	a[3] = -31;
//	a[4] = 0;
//	a[5] = 99;
//	a[6] = 2;
//	a[7] = 83;
//	a[8] = 782;
//	a[9] = 1;
//	int n = 10;
//	int i;
//	for (i = 0; i < n; i++)
//		if(i == n-1) printf("%d%s", a[i], "\n");
//		else printf("%d%s", a[i], " ");
//	bubble_sort(a, n);
//	for (i = 0; i < n; i++)
//		if(i == n-1) printf("%d%s", a[i], "\n");
//		else printf("%d%s", a[i], " ");
//	return 0;
//}
//

int a[5];

int main()
{
	int b = a+5;
	b = a+b;
	return b;
}