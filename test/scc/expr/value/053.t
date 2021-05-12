void test()
{
	struct S { int a; };
	__offsetof(struct S, a); // rvalue
}