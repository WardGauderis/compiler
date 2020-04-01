const int f = 6;
int g(int* f){
	return ++*f;
}

int main()
{
	printf(g(&f));
}


//int f = 6;
//
//void g(const int* f)
//{
//	++(*f);
//}
//
//int main()
//{
//	g(&f);
//}
