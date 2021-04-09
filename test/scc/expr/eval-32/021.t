struct A
{
	int a;
	char b[0];
};

void test()
{
	sizeof(struct A); // 4U
}