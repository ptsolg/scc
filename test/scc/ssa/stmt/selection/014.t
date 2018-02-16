void test()
{
	int a;
	switch (a)
	{
		label:
			a = 101;
			break;

		case 10:
			goto label;
	}
}