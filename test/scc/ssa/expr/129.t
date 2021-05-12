struct S
{
	char a[3];
	struct
	{
		int b;
	};
	int c;
};

unsigned test()
{
	return __offsetof(struct S, b);
}