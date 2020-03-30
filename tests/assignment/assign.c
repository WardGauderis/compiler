int f = 6;

void f(){
    printf(f);
}

int f(){
    return 3;
}

int main()
{
    int x = 10 * f();
    printf(x);
    f();
}

