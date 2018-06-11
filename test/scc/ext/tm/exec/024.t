#include "thread.h"
#include "memory.h"

int main()
{
	char* a = xmalloc(40, 4);
	for (int i = 0; i < 40; i++)
		a[i] = 1;

	_Atomic
	{
		*(long long int*)(a + 11) = 0;
		*(long long int*)(a + 11) = 72340172838076673ULL; // restore
	}

	for (int i = 0; i < 40; i++)
		if (a[i] != 1)
			return 3;

	return 0;
}