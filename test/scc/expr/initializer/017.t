typedef struct
{
	int x;
	int y;
} Point;

typedef struct
{
	Point begin;
	Point end;
} Line;

void test()
{
	Line l = { {0, 0, }, { 1, 1, }, };
}