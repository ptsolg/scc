int test()
{
	int iterator = 0;
	int end = 100;

	while(iterator != end)
	{
		if (iterator == 10)
		{
			if (!iterator)
				continue;
			else
				break;
		}
		else
			return 0;
	}	

	iterator++;
}