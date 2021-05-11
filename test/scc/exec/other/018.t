struct S
{
	int a;
};

struct S f()
{
	struct S a = { 10 };
	return a;
}

int main()
{
	struct S a = f();
	return a.a - 10;
}