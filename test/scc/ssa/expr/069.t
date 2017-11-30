int* foo()
{
	int* a;
	return a;
}

void test()
{
	int a = foo()[1];
}