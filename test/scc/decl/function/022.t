void foo( void func() )
{
}

void bar()
{
	void(*ptr)();
	foo(ptr);
}