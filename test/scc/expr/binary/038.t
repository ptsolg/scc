struct A
{
	struct B* b;
};

struct B
{
	int c;
};

void test(struct A* a)
{
	a->b->c;
}