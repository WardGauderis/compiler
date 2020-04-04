

int   f(int, int);
int   f(int, int);   // no error
int   f(float, int); // error
float f(int, int);   // error

int f(int a, int b)
{
    return a + b * 2;
}

int main()
{
    return f(5, 5);
}
