struct A { int x; };
struct B { int y; };

void foo(struct A a)
{
}

void test()
{
	struct B b;
    foo(b);
}