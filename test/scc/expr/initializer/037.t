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
	} f = { .b.a = 1, 2, .b.a = 2, .e.c = 3, 4 };
}			