int g(int* f)
{
	return (*f)++;
}

int main()
{
	int f = 10;
	printf(g(&(f)));
	printf(f);
}


const int f = 6;

void g(int* f)
{
	++(*f);
}

int main()
{
	g(&f);
	printf(f);
}
