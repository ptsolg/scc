void test()
{
	struct
	{
		struct
		{
			int a;
		} b;

		struct
		{
			int c, b;
		} e;
	} f = { .b.a = 1, 2, 3 };
}			