void test()
{
	for (int i = 0; i < 10; i++)
	{
		for (int j = 0; j < 10; j++)
		{
			if (j == 10)
				break;

			j = 20;

			if (j == 20)
				continue;
		}
	}
}