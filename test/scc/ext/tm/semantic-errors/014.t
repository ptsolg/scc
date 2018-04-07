void test()
{
    struct
    {
        volatile int a;
    } *b;
    _Atomic
        b->a;
}