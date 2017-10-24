typedef struct
{
	struct
	{
		int b;
	} a;

	struct
	{
		struct
		{
			int d;
			int e;
		} c;

		int f;
	} b;

} A;

void test()
{
	A a =
	{
		.a = { 10 + 20 - 30, },
		.b = { { 100, 200, }, 300, },
	};
}