struct A
{
	char a[10];
	int b;
	char c[3];
};

int main()
{
	return __offsetof(struct A, b) - 10;
}