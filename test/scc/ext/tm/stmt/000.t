void test()
{
	_Atomic
	{
		int a = 1;
		if (a)
			;
	}
}