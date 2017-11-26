void foo(int n, ...)
{
}

void test()
{
	char a;
	unsigned char b;
	short c;
	foo(10, a, b, c);
}