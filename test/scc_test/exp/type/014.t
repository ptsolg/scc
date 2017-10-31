union A 
{
	const void* b;
};

void test(int a, int b)
{
	union A* a;
	a->b; // const void*
}