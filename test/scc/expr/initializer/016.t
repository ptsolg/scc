typedef struct
{
	struct 
	{
		struct 
		{
			struct
			{
				int a;
				int b;
			} c;

			int g[10];
		} d;

		int e;
	} f[10];
} Object;

void test()
{
	Object o =
	{
		.f[0].e      = 1,
		.f[1].d.g[0] = 2,
		.f[2].d.c.a  = 3,
	};
}