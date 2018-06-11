#include "thread.h"
#include "memory.h"

struct Node
{
	struct Node* next;
	int val;
};

struct Node* node;

void inc1(struct Node* node) _Transaction_safe
{
	node->val += 2;
}

void inc2(struct Node* node) _Transaction_safe
{
	node->next->val++;
}

void inc4(struct Node* node) _Transaction_safe
{
	node->next->next->next->val++;
}

void entry(void* data)
{
	for (int i = 0; i < 10000; i++)
	{
		_Atomic
		{
			inc1(node);
			node->val -= 1;
			inc2(node);
			node->next->next->val += 2;
			inc4(node);
			node->next->next->val -= 1;
		}
	}
}

int main()
{
	node = (struct Node*)((char*)xmalloc(sizeof(struct Node) * 4 + 10, 4) + 2);
	for (int i = 0; i < 3; i++)
	{
		node[i].val = 0;
		node[i].next = &node[i + 1];
	}
	node[3].val = 0;
	node[3].next = 0;

	CREATE_THREADS(2, entry, 0);

	for (int i = 0; i < 4; i++)
		if (node[i].val != 20000)
			return 1;

	return 0;
}