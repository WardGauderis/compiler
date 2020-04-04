int main()
{
	int x[10][5];
	int y = 0;
	for (int i = 0; i<5; ++i)
	{
		for (int j = 0; j<10; ++j)
		{
			x[i][j] = i+j;
		}
	}
	for (int i = 0; i<5; ++i)
	{
		for (int j = 0; j<10; ++j)
		{
			y = y + x[i][j];
		}
	}
	return y;
}
