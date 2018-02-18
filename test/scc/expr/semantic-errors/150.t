void test()
{
    union
    {
        int a;
        struct
        {
            int b, c;
        } d;
        int e;
    } c = { .d.b = 1, 2, 3 };
}