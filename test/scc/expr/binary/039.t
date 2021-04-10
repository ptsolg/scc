typedef struct B* BPTR;
typedef struct A* APTR;

struct A
{
	BPTR b;
};

struct B
{
	int c;
};

void test(APTR a)
{
	a->b->c;
}