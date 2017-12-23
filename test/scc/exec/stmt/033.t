int main()
{
	int c = 0;
	for (int i = 0; i < 10; i++)
	{
		if (i % 2 == 0)
			c++;
		else
			continue;
		c += 100;
	}

	if (c == 505)
		return 0;
	return 0;
}