void test()
{
	int a;
	switch (a)
	{
		case 10:
		case 20:
			a = 1;
		case 30:
			a = 2;
		default:
			a = 3;
	}
}