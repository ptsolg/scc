int main()
{
	int c = 0;
	unsigned j = 0;
	
	while (j < 10)
	{
		char k = 0;
		while (k < 10ULL)
		{
			c++;
			k++;
		}
		j++;
	}

	if (c == 100)
		return 0;
	return 1;
}