int foo(void)
{
	return 10;
}

int main()
{
	return !(foo() == 10);
}