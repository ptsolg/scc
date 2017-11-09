struct A
{
	int a;
};

struct B
{
	struct A a;
	int bb;
};

struct C
{
	struct B b;
	int cc;
};

void test()
{
	struct C c = { { { 1, }, 2, }, 3, };
}