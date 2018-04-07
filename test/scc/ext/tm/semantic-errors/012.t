void test() _Transaction_safe
{
    struct
    {
        volatile int a;
    } *b;
    b->a;
}