#include "thread.h"

int a;

void inc()
{
	_Atomic
	{
		a += 2;
		_Atomic
		{
			a -= 1;
			return;
		}
	}
}

void entry(void* data)
{
	for (int i = 0; i < 10000; i++)
		inc();
}

int main()
{
	CREATE_THREADS(2, entry, 0);
	return !(a == 20000);
}