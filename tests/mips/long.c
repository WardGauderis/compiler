#include <stdio.h>

int main()
{
	int a = 0;
	int b = !0;
	int d = +0;
	int c = +0;

	int e = ++a;
	int f = --a;
	int g = a++;
	int h = a--;

	int i = --a;
	int j = !-!1;
	int k = -1-1-1;
	int l = -+1+!!!4;

	int m = --l;
	int n = l--;
	int o = b != c;
	int p = ++b < --c;
	int q = d * e;
	int r = g < h;
	int s = n > m;
	int t = (g = h);
	int u = (k = k == l);
	int v = r != s;
	int w = (int)(char)(int)'a';

	int x = 80 % 2;
	int y = 80 / 2;
	int z = 2 * 400;


	printf("%i\n", a);
	printf("%i\n", b);
	printf("%i\n", c);
	printf("%i\n", d);
	printf("%i\n", e);
	printf("%i\n", f);
	printf("%i\n", g);
	printf("%i\n", h);
	printf("%i\n", i);
	printf("%i\n", j);
	printf("%i\n", k);
	printf("%i\n", l);
	printf("%i\n", m);
	printf("%i\n", n);
	printf("%i\n", l);
	printf("%i\n", o);
	printf("%i\n", p);
	printf("%i\n", q);
	printf("%i\n", r);
	printf("%i\n", s);
	printf("%i\n", t);
	printf("%i\n", u);
	printf("%i\n", v);
	printf("%i\n", w);
	printf("%i\n", x);
	printf("%i\n", y);
	printf("%i\n", z);
}