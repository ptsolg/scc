#include "thread.h"

struct
{
	int a;
	int b;
} c;

void inc(int* a, int* b) _Transaction_safe
{
	*a += 1;
	*b += 1;
}

void entry(void* data)
{
	for (int i = 0; i < 10000; i++)
	{
		_Atomic
			inc(&c.a, &c.b);
	}
}

int main()
{
	CREATE_THREADS(2, entry, 0);

	if (c.a != 20000)
		return 1;
	if (c.b != 20000)
		return 1;

	return 0;
}