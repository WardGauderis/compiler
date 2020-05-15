int n = 10;

int main()
{
    if(n <= 1) return 1;
    else
    {
        int t = n;
        n = n - 1;
        return main() * t;
    }
}
