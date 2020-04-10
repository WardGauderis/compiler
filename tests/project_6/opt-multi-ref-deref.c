
int main()
{
	int* a[10];
	int b = a +5;

	int *c[10][5];
	int d = *&(5+c)[2];
}
