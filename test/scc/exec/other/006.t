int main()
{
	extern int foo();
	return foo();
}

extern int foo()
{
	return 0;
}