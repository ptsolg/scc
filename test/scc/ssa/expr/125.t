struct file_lookup
{
	struct vec* v;
};

struct vec
{
	int size;
};

void test(struct file_lookup* fl)
{
	fl->v->size = 0;
}