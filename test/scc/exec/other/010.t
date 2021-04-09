#include <stdlib.h>

typedef struct
{
	int size;
	char data[];
} _Chunk;

typedef struct
{
	int kind;
	union
	{
		_Chunk c1;
		_Chunk c2;
	} c;
} Chunk;

int main()
{
	Chunk* c = malloc(sizeof(Chunk) + 1024);
	for (int i = 0; i < 1024; i++)
		c->c.c1.data[i] = 123;
	free(c);
	return 0;
}