struct A
{
	int a;
};

void test()
{
	__offsetof(struct A, a); // 0U
}