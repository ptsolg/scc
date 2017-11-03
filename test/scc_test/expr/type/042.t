void test()
{
	const int* a;
	volatile void* b;

	1 ? b : a; // const volatile void*
}