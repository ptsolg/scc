typedef void* pvoid;

void foo(pvoid p)
{
}

void test()
{
	int a;
	foo(&a);
}