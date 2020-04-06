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


    printf("%i", a);
    printf("%i", b);
    printf("%i", c);
    printf("%i", d);
    printf("%i", e);
    printf("%i", f);
    printf("%i", g);
    printf("%i", h);
    printf("%i", i);
    printf("%i", j);
    printf("%i", k);
    printf("%i", l);
    printf("%i", m);
    printf("%i", n);
    printf("%i", l);
    printf("%i", o);
    printf("%i", p);
    printf("%i", q);
    printf("%i", r);
    printf("%i", s);
    printf("%i", t);
    printf("%i", u);
    printf("%i", v);
    printf("%i", w);
    printf("%i", x);
    printf("%i", y);
    printf("%i", z);
}