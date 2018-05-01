#include <thread.h>

#define NTHREADS 10
#define NITER 100000

struct mutex
{
	unsigned val;
};

static void mutex_init(struct mutex* self)
{
	self->val = 0;
}

static void mutex_lock(struct mutex* self)
{
	while (!__atomic_cmpxchg_32_weak_seq_cst(&self->val, 0, 1))
		;
}

static void mutex_release(struct mutex* self)
{
	self->val = 0;
}

struct mutex m;
int c;

void entry(void* data)
{
	for (int i = 0; i < NITER; i++)
	{
		mutex_lock(&m);
		c++;
		mutex_release(&m);
	}
}

int main()
{
	c = 0;
	mutex_init(&m);

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