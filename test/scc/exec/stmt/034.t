int main()
{
	int c = 0;
	for (int i = 0; i < 10; i++)
	{
		for (int j = 0; j < 10; j++)
		{
			c++;
			break;
		}
		c++;
	}

	if (c == 20)
		return 0;
	return 1;
}