volatile int* foo() _Transaction_safe
{
    return 0;
}

void test() _Transaction_safe
{
    *foo() = 2;
}