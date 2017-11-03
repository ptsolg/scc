struct A
{
	int a;
};

struct B 
{
	struct A* a;
};

void test(int a, int b)
{
	struct B* b;
	b->a->a; // int
}