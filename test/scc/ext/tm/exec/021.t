#include "thread.h"
#include "memory.h"

int main()
{
	int* a = xmalloc(4, 4);
	*a = 0;
	char* ca = (char*)a;
	int b = 0;

	_Atomic
	{
		ca[0] = 100;
		b = *a;
	}

	if (b != 100)
		return 1;

	return 0;
}