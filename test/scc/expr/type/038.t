void test()
{
	const int* a;
	volatile int* b;
	1 ? b : a; // const volatile int*
}