void test()
{
        struct Node
        {
                enum Kind
                {
                        NK_A = 0,
                        NK_B = 2,
                        NK_C = 4,
                } kind;

                struct Node* next;
                struct Node* prev;

        } node;
}