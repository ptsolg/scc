int test()
{
	int a;
	switch(a)
	{
		case 9:
		case 10:
			break;

		case 11:
			return 10;

		default:
			return 0;
	}
}