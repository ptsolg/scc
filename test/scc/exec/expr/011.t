static int foo(int a, ...)
{
	return a;
}

int main()
{
	return foo(0, 1, 2, 3, 4);
}