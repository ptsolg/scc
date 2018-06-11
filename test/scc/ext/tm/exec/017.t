#include "thread.h"

int**** p4;

void entry(void* data)
{
	for (int i = 0; i < 10000; i++)
	{
		_Atomic
		{
			****p4 += 1;
			*((***p4) + 1) += 1;
		}
	}
}

int main()
{
	int a[2];
	a[0] = 0;
	a[1] = 0;

	int* p1 = &a[0];
	int** p2 = &p1;
	int*** p3 = &p2;
	p4 = &p3;

	CREATE_THREADS(2, entry, 0);

	if (a[0] != 20000)
		return 1;
	if (a[1] != 20000)
		return 1;

	return 0;
}