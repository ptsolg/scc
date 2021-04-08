int foo() {
	return 123;
}

typedef int(*foo_fn)();

struct {
	foo_fn f;
} a = { .f = &foo };

int main()
{
	return a.f() - 123;
}