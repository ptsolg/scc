typedef struct
{
	int items[3];
} Buf;

void test()
{
	Buf b = { { [1] = 1, 2 } };
}