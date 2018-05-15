#include "thread.h"

int a;

void entry(void* data)
{
	for (int i = 0; i < 10000; i++)
	{
		_Atomic
		{
			a += 2;
			_Atomic
			{
				a += 1;
				_Atomic
				{
					a += 1;
					a -= 1;
				}
				a -= 1;
			}
			a -= 1;
		}
	}
}

int main()
{
	CREATE_THREADS(2, entry, 0);
	if (a != 20000)
		return 1;
	return 0;
}