void test()
{
	struct
	{
		struct
		{
			int m1;
			int m2;
		} s2;
	} s1;

	int a = s1.s2.m2;
}