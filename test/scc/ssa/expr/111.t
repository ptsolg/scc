struct S
{
	int a;
};

void test()
{
	struct S** a;
	(*a)->a;
}