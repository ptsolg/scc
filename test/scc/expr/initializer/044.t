struct A
{
	struct { int a; };
};

void test()
{
    struct
    {
        struct
        {
            struct A b;
        };
    } b = { .b.a = 10 };
}