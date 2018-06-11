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
		a[12] += 2;
		a[13] += 1;
		a[14] = 1;
		a[12] -= 1;
	}

	for (int i = 0; i < 40; i++)
		if (a[i] != 1)
			return 1;

	return 0;
}