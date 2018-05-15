#include <thread.h>

unsigned c;

void entry(void* data)
{
	for (int i = 0; i < 100000; i++)
		__atomic_add_fetch_32_seq_cst(&c, 1);
}

int main()
{
	CREATE_THREADS(2, entry, 0);
	return !(c == 200000);
}