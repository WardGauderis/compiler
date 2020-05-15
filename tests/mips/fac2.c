int fib(int val)
{
    if(val <= 1) return 1;
    else return val * fib(val - 1);
}

int main()
{
    return fib(10);
}