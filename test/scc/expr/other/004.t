typedef int i32;

void foo(const i32* a)
{
}

void test()
{
	const void* a;
	foo(a);
}