#include "thread.h"
#include "memory.h"

int main()
{
	char* a = xmalloc(40, 4);
	for (int i = 0; i < 40; i++)
		a[i] = 1;

	_Atomic
	{
		*(int*)(a + 11) = 0;
		*(int*)(a + 11) += 1;
	}

	int i = 0;
	for (; i < 12; i++)
		if (a[i] != 1)
			return 1;
	for (; i < 15; i++)
		if (a[i] != 0)
			return 2;
	for (; i < 40; i++)
		if (a[i] != 1)
			return 3;

	return 0;
}