#include "thread.h"
#include "memory.h"

int* vars[4];

void entry(void* data)
{
	for (int i = 0; i < 10000; i++)
	{
		_Atomic
		{
			for (int i = 0; i < 4; i++)
				*vars[i] += 1;
			for (int i = 0; i < 4; i++)
				*vars[i] = *vars[(i + 1) % 4];
		}
	}
}

int main()
{
	for (int i = 0; i < 4; i++)
	{
		if (!(vars[i] = xmalloc(sizeof(int), i + 1)))
			return 10;
		*vars[i] = 0;
	}

	CREATE_THREADS(2, entry, 0);

	for (int i = 0; i < 4; i++)
		if (*vars[i] != 20000)
			return 1;

	return 0;
}