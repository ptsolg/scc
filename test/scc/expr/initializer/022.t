struct A
{
	int a;
};

int foo()
{
	return 10;
}

void test()
{
	struct A b = { foo() };
}