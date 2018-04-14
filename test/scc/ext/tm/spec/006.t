void test()
{
	void(*foo)() _Transaction_safe;
	void(*bar)() _Transaction_safe;
	foo = bar;
}