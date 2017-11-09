struct A
{
	int a;
};

struct B
{
	struct A a;
};

struct C
{
	struct B b;
};

void test()
{
	struct C c = { { { 1, }, }, };
}