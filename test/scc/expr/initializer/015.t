typedef struct
{
	int x;
	int y;


	int v[10];
} Object;

void test()
{
	Object a[10][20] = 
	{
		[0][1].x = 0,
		[1][2].v[0] = 1,
	};
}