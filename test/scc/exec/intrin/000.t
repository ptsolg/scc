#include <thread.h>

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
	for (int i = 0; i < 100000; i++)
	{
		mutex_lock(&m);
		c++;
		mutex_release(&m);
	}
}

int main()
{
	mutex_init(&m);
	CREATE_THREADS(2, entry, 0);
	return !(c == 200000);
}