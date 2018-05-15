#include "thread.h"

int c;

void entry(void* data)
{
	for (int i = 0; i < 1000; i++)
	{
		_Atomic
			for (int j = 0; j < 10; j++)
				c++;
	}
}

int main()
{
	CREATE_THREADS(2, entry, 0);
	return !(c == 20000);
}