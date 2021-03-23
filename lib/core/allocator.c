#include "scc/core/allocator.h"
#include "scc/core/alloc.h"

void init_stack_alloc(struct stack_alloc* self)
{
        list_init(&self->chunks);
        self->chunk_pos = 0;
        self->chunk_end = 0;
}

void drop_stack_alloc(struct stack_alloc* self)
{
        while (!list_empty(&self->chunks))
                dealloc(list_pop_front(&self->chunks));
}

void stack_alloc_grow(struct stack_alloc* self, size_t at_least)
{
        struct
        {
                list_node node;
                char data[0];
        } *chunk;

        size_t min_size = 4096 - sizeof(*chunk);
        size_t data_size = at_least < min_size ? min_size : at_least;

        chunk = alloc(sizeof(*chunk) + data_size);
        self->chunk_pos = chunk->data;
        self->chunk_end = chunk->data + data_size;
        list_push_back(&self->chunks, &chunk->node);

}

void init_object_pool(struct object_pool* self, size_t object_size)
{
        init_stack_alloc(&self->stack);
        self->top = 0;
        self->object_size = (unsigned)object_size;
}

void drop_object_pool(struct object_pool* self)
{
        drop_stack_alloc(&self->stack);
}