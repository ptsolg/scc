struct S
{
	int a;
};

void test(const struct S* a)
{
	struct S b = *a;
}