#include <thread.h>

#define NTHREADS 10
#define NITER 100000

unsigned c;

void entry(void* data)
{
	for (int i = 0; i < NITER; i++)
		__atomic_add_fetch_32_seq_cst(&c, 1);
}

int main()
{
	c = 0;
	struct thread ths[NTHREADS];

	for (int i = 0; i < NTHREADS; i++)
	{
		thread_init(ths + i, entry, 0);
		thread_start(ths + i);
	}
	for (int i = 0; i < NTHREADS; i++)
		thread_wait(ths + i);

	return !(c == NTHREADS * NITER);
}