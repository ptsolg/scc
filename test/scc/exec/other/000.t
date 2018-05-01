#include <thread.h>

#define NTHREADS 10
#define NITER 100000

_Thread_local int c;

void entry(void* data)
{
	for (int i = 0; i < NITER; i++)
		c++;
	*(int*)data = c;
}

int main()
{
	struct thread ths[NTHREADS];
	int res[NTHREADS];

	for (int i = 0; i < NTHREADS; i++)
	{
		thread_init(ths + i, entry, res + i);
		thread_start(ths + i);
	}
	for (int i = 0; i < NTHREADS; i++)
		thread_wait(ths + i);

	for (int i = 0; i < NTHREADS; i++)
		if (res[i] != NITER)
			return 1;

	return 0;
}