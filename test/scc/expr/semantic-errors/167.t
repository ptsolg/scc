void test()
{
    struct S
    {
        struct V* a;
    } b;
    struct V
    {
        int b;
    };
    b.a->a;
}

struct V
{
    int a;
};