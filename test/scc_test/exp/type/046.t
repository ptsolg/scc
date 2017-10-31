void test()
{
	int* a;
	1 ? a : (const void*)0; // const int*
}