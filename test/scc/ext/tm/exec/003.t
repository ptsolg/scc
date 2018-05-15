#include "thread.h"

int c;

void entry(void* data)
{
	for (int i = 0; i < 10000; i++)
		_Atomic
		{
			c += 2;
			c -= 1;
		}
}

int main()
{
	CREATE_THREADS(2, entry, 0);
	return !(c == 20000);
}