void test()
{
    void(***foo)();
    void(***bar)() _Transaction_safe;
    foo = bar;
}