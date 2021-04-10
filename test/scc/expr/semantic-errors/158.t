struct A
{
    union B* a;
};

struct B
{
    int a;
} b;

void test(struct A* a)
{
    a->a = &b;
}