void foo()
{
}

void test()
{
    void(*p)();
    p = foo;
}