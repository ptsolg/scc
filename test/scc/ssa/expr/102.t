void test()
{
	struct
	{
		int a;

		struct
		{
			int m1;
			int m2;
			int m3;
		};
	} s1;

	int a = s1.m2;
}
