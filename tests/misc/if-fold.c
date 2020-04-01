
int main()
{
    const int a = 5;
    if(a)
    {
        printf(6);
    }

    if(4)
    {
        printf(a);
    }
    else
    {
        printf(777);
    }

    if(0)
    {
        printf(555555); // this will disappear
    }
}