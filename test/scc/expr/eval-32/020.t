struct A
{
	int a;
	char b[];
};

void test()
{
	sizeof(struct A); // 4U
}