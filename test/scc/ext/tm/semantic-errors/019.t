void test()
{
    void(*a)();
    void(*b)() _Transaction_safe = a;
}