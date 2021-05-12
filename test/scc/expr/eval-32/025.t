struct A
{
	int a;
	struct
	{
		int b;
		struct
		{
			int c;
		};
	};
};

void test()
{
	__offsetof(struct A, c); // 8U
}