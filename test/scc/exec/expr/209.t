int foo() {
	return 123;
}

typedef int(*foo_t)();
typedef foo_t arr_t[3];

arr_t a = { foo, &foo, foo };

int main()
{
	return a[0]() - a[1]() + a[2]() - 123;
}