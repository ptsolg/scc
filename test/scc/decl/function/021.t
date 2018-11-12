typedef int((array_t))[10];

void foo(array_t array)
{
}

void bar()
{
	int* a;
	foo(a);
}