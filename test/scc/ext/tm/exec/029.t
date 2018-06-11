#include "thread.h"
#include "memory.h"

int* a;

void entry(void* data)
{
	for (int i = 0; i < 1000; i++)
		_Atomic
		{
			for (int i = 0; i < 32; i++)
				a[i] = a[i] + 1;
			for (int i = 0; i < 32; i++)
				a[i] = a[i];
		}
}

int main()
{
	a = (int*)((char*)xmalloc(sizeof(int) * 32 + 10, 4) + 2);
	for (int i = 0; i < 32; i++)
		a[i] = 0;
	
	CREATE_THREADS(2, entry, 0);
	for (int i = 0; i < 32; i++)
		if (a[i] != 2000)
			return 1;

	return 0;
}