void test()
{
    struct
    {
        int a;
        struct
        {
            int b, c;
        } d;
    } e = { .d = { 0, 1 }, 3};
}