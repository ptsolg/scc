struct s
{
    int a;
};

void foo(void* p)
{
}

void test()
{
    foo(&s);
}