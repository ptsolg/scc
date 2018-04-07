volatile int* foo() _Transaction_safe
{
    return 0;
}

void test()
{
    _Atomic
        *foo() = 2;
}