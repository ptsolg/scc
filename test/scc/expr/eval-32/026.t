typedef struct
{
	char a[4];
	int b;
} Struct;

void test()
{
	__offsetof(Struct, b); // 4U
}