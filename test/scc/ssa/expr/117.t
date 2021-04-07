typedef int int32;
typedef int array[3];

typedef struct
{
	array items;
} Buf;

void test()
{
	Buf b = { { [1] = 1, 2 } };
}