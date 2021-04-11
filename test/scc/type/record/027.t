struct A
{
	int a;
	union B* b[];
};

union B
{
	struct A a;
	int b;
};