#include "thread.h"

int c;

void entry(void* data)
{
	for (int i = 0; i < 10000; i++)
		_Atomic
			_Atomic
				c++;
}

int main()
{
	CREATE_THREADS(2, entry, 0);
	return !(c == 20000);
}