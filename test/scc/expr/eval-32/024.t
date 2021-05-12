union A
{
	int a;
	int b;
};

void test()
{
	__offsetof(union A, b); // 0U
}