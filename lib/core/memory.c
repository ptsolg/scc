#include "scc/core/memory.h"
#include "scc/core/malloc.h"
#include <setjmp.h>

static void* mallocate(void* self, size_t bytes)
{
        return core_malloc(bytes);
}

static void mdeallocate(void* self, void* block)
{
        core_free(block);
}

extern allocator* _get_stdalloc()
{
        static allocator stdalloc = { &mallocate, &_allocate_aligned, &mdeallocate };
        return &stdalloc;
}

extern void mallocator_init(allocator* self)
{
        allocator_init(self, &mallocate, &mdeallocate);
}

extern void obstack_init(obstack* self)
{
        obstack_init_ex(self, STDALLOC);
}

extern void obstack_init_ex(obstack* self, allocator* alloc)
{
        self->alloc = alloc;
        self->chunk.pos = NULL;
        self->chunk.end = NULL;
        self->chunk.size = 0;
        allocator_init_ex(&self->base,
                &obstack_allocate,
                &obstack_allocate_aligned,
                &obstack_deallocate);
        list_init(&self->chunks);
}

extern void obstack_dispose(obstack* self)
{
        while (!list_empty(&self->chunks))
                deallocate(self->alloc, list_pop_front(&self->chunks));
}

extern errcode obstack_grow(obstack* self, size_t at_least)
{
        struct
        {
                list_node node;
                uint8_t data[0];
        } *chunk;

        size_t data_size = at_least + self->chunk.size;
        if (!(chunk = allocate(self->alloc, sizeof(*chunk) + data_size)))
                return EC_ERROR;

        self->chunk.pos = chunk->data;
        self->chunk.end = self->chunk.pos + data_size;
        self->chunk.size = data_size;
        list_push_back(&self->chunks, &chunk->node);
        return EC_NO_ERROR;
}

extern void objpool_init(objpool* self, size_t obsize)
{
        objpool_init_ex(self, obsize, STDALLOC);
}

extern void objpool_init_ex(objpool* self, size_t obsize, allocator* alloc)
{
        obstack_init_ex(&self->base, alloc);
        self->obsize = obsize;
        self->top = NULL;
}

extern void objpool_dispose(objpool* self)
{
        obstack_dispose(&self->base);
}

extern void mempool_init(
        mempool* self, mempool_bad_alloc_handler handler, jmp_buf on_bad_alloc)
{
        mempool_init_ex(self, handler, on_bad_alloc, STDALLOC);
}

extern void mempool_init_ex(
        mempool* self, mempool_bad_alloc_handler handler, jmp_buf on_bad_alloc, allocator* alloc)
{
        assert(on_bad_alloc);
        self->alloc = alloc;
        self->handler = handler;
        self->on_bad_alloc = on_bad_alloc;
        allocator_init(&self->base, &mempool_allocate, &mempool_deallocate);
        list_init(&self->used);
}

extern void mempool_dispose(mempool* self)
{
        while (!list_empty(&self->used))
                deallocate(self->alloc, list_pop_front(&self->used));
}
