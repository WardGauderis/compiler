int main()
{
    char*  a = 1;
    float* b = a;

    a = 2 + b;

    b = a + 2;

    printf(a);
    printf(b);
}