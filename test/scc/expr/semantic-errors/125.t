void test()
{
    struct A { int a; };
    struct B { char c; } b;
    struct { struct A a; } c = { b };
}