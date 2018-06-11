#include "thread.h"
#include "memory.h"

int* a;

void entry(void* data)
{
	for (int i = 0; i < 10; i++)
		_Atomic
		{
			for (int i = 0; i < 64; i++)
				a[i] += 1;
		}
}

int main()
{
	a = (int*)((char*)xmalloc(sizeof(int) * 64 + 10, 4) + 3);
	for (int i = 0; i < 64; i++)
		a[i] = 0;
	
	CREATE_THREADS(2, entry, 0);
	for (int i = 0; i < 64; i++)
		if (a[i] != 20)
			return 1;

	return 0;
}