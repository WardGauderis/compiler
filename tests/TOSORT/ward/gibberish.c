int main()
{
    int* a[10][5];
    int ** b = &*a[3];
    int * c = (&((*a)[3]))[2];
    int d = &((&c[3])[3]);
    int e = *&*(5+a)[2];
}
