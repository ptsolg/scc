struct S
{
	int a;
};

struct S foo()
{
	struct S a;
	return a;
}

int bar()
{
	return foo().a;
}