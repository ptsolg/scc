#include "thread.h"

struct Node
{
	struct Node* next;
	int val;
};

struct Node* node;

void entry(void* data)
{
	for (int i = 0; i < 10000; i++)
	{
		_Atomic
		{
			node->val++;
			node->next->val++;
			node->next->next->val++;
			node->next->next->next->val++;
		}
	}
}

int main()
{
	struct Node nodes[4];
	for (int i = 0; i < 3; i++)
	{
		nodes[i].val = 0;
		nodes[i].next = &nodes[i + 1];
	}
	nodes[3].val = 0;
	nodes[3].next = 0;
	node = &nodes[0];

	CREATE_THREADS(2, entry, 0);

	for (int i = 0; i < 4; i++)
		if (nodes[i].val != 20000)
			return 1;

	return 0;
}