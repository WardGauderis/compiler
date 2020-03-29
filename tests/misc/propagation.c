// constant folding and constant propagation

int main()
{
    const int a = 5;
    const int b = 6;
    int       c = a + b;

    int d = 5;
    int e = 6;
    int f = d + e;

    (4 + 6) / 2;
    (3 + 5) / 0; //! will not fold
    printf(a);
    printf(b);
    printf(c);
    printf(d);
    printf(e);
    printf(f);
}