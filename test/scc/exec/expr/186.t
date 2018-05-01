int foo()
{
	return 10;
}

int bar(int(*foo)())
{
	return foo();
}

int main()
{
	if (bar(foo) != 10)
		return 10;
	return 0;
}