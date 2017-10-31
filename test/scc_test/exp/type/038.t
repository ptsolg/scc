void test()
{
	const int* a;
	restrict volatile int* b;
	1 ? b : a; // const restrict volatile int*
}