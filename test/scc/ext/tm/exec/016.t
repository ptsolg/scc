#include "thread.h"

int**** p4;

void entry(void* data)
{
	for (int i = 0; i < 10000; i++)
	{
		_Atomic
			****p4 += 1;
	}
}

int main()
{
	int a = 0;
	int* p1 = &a;
	int** p2 = &p1;
	int*** p3 = &p2;
	p4 = &p3;

	CREATE_THREADS(2, entry, 0);

	return !(a == 20000);
}