typedef struct
{
	int x;
	int y;
} Point;

typedef Point Point2D;

typedef struct 
{
	Point2D p1;
	Point2D p2;
} Line;

typedef Line Line2D;

void test()
{
	Line2D lines[10] = 
	{
		[0] = { { .x = 0, .y = 1, }, { 2, 3, }, },
	};
}