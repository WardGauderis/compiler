int f = 6;

void h(){
    printf(f);
}

int g(){
    return 3;
}

int main()
{
    int x = 10 * g();
    printf(x);
    h();
}

