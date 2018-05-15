#include "thread.h"

int a;
int b;

void entry(void* data)
{
	for (int i = 0; i < 10000; i++)
	{
		_Atomic
		{
			a++;
			_Atomic
				b++;
		}
	}
}

int main()
{
	CREATE_THREADS(2, entry, 0);
	if (a != 20000)
		return 1;
	if (b != 20000)
		return 2;
	return 0;
}