int main()
{
	int c = 0;
	int i = 0;
	do
	{
		int j = 0;
		do
		{
			c++;
			j++;
		} while(j < 10);

		i++;
	} while (i < 10);

	if (c == 100)
		return 0;
	return 1;
}