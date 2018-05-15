#include "thread.h"

int c;

void entry(void* data)
{
	for (int i = 0; i < 1000; i++)
	{
		int j = 0;
		_Atomic
			do
			{
				c++;
				j++;
			} while (j < 10);
	}
}

int main()
{
	CREATE_THREADS(2, entry, 0);
	return !(c == 20000);
}