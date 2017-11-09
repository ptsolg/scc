typedef struct
{
	struct 
	{
		int b;
		int c;
	} a;

	struct 
	{
		struct
		{
			int f;
		} e;
	} d;

} Object;

void test()
{
	Object o = { .a.b = 0, .a.c = 1, .d.e.f = 2, };
}