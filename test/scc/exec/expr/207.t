int foo() {
	return 123;
}

int(*a)() = foo;
int(*b)() = &foo;

int main()
{
	return a() - b();
}