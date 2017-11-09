void test()
{
	int* a;
	1 ? (const void*)0 : a; // const int*
}