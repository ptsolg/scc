struct A
{
	int a;
	int b;
};

void test()
{
	__offsetof(struct A, b); // 4U
}