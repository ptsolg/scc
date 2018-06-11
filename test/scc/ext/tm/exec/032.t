#include "thread.h"
#include "memory.h"

struct Node
{
	struct Node* next;
	int val;
};

struct List
{
	struct Node* head;
};

struct List list;

void inclist(struct List* list) _Transaction_safe
{
	struct Node* it = list->head;
	while (it)
	{
		it->val++;
		it = it->next;
	}
}

void entry(void* data)
{
	for (int i = 0; i < 10000; i++)
		_Atomic
			inclist(&list);
}

int main()
{
	struct Node* nodes = xmalloc(sizeof(struct Node) * 50, 3);
	for (int i = 0; i < 49; i++)
	{
		nodes[i].val = 0;
		nodes[i].next = &nodes[i + 1];
	}
	nodes[49].val = 0;
	nodes[49].next = 0;
	list.head = &nodes[0];

	CREATE_THREADS(2, entry, 0);

	for (int i = 0; i < 50; i++)
		if (nodes[i].val != 20000)
			return 1;

	return 0;
}