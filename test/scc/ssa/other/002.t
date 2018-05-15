typedef struct _Node
{
	struct _Node* prev;
	struct _Node* next;
} Node;

typedef struct
{
	Node* head;
	Node* tail;
	int size;
} List;

void test(List* self, Node* node)
{
	self->head = node;
}
