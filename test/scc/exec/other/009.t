#include <stdlib.h>

struct chunk
{
	int a;
	char b[];
};

int main()
{
	struct chunk* c = malloc(sizeof(struct chunk) + 1024);
	for (int i = 0; i < 1024; i++)
		c->b[i] = 123;
	free(c);
	return 0;
}