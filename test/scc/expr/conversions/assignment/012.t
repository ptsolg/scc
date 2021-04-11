struct A
{
	int a;
};

void test(const struct A* a)
{
	struct A b = *a;
}