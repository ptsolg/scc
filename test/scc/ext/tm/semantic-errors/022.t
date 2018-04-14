void foo(void(*bar)() _Transaction_safe)
{
}

void test()
{
    void(*a)();
    foo(a);
}