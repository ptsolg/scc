struct A
{
	int a, b, c;
};

void test()
{
	__offsetof(struct A, c); // 8ULL
}