void test()
{
	const int* a;
	volatile int* b;
	1 ? a : b; // const volatile int*
}