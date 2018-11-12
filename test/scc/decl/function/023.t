typedef void func_t();

void foo(func_t func)
{
}

void bar()
{
	void(*ptr)();
	foo(ptr);
}