#include "thread.h"

int a;

void inc(int n) _Transaction_safe
{
	if (n <= 0)
		return;

	a++;
	inc(n - 1);
}

void entry(void* data)
{
	for (int i = 0; i < 1000; i++)
		_Atomic
			inc(10);
}

int main()
{
	CREATE_THREADS(2, entry, 0);
	return !(a == 20000);
}