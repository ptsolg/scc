#include "scc/scl/memory.h"
#include "scc/scl/malloc.h"
#include <setjmp.h>

static void* mallocate(allocator* self, ssize bytes)
{
        return smalloc(bytes);
}

static void mdeallocate(void* self, void* block)
{
        sfree(block);
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
        self->_alloc = alloc;
        self->_chunk.pos = NULL;
        self->_chunk.end = NULL;
        self->_chunk.size = 0;
        allocator_init_ex(&self->_base,
                &obstack_allocate,
                &obstack_allocate_aligned,
                &obstack_deallocate);
        list_init(&self->_chunks);
}

extern void obstack_dispose(obstack* self)
{
        while (!list_empty(&self->_chunks))
                deallocate(self->_alloc, list_pop_front(&self->_chunks));
}

extern serrcode obstack_grow(obstack* self, ssize at_least)
{
        struct
        {
                list_node node;
                suint8 data[0];
        } *chunk;

        ssize data_size = at_least + self->_chunk.size;
        if (!(chunk = allocate(self->_alloc, sizeof(*chunk) + data_size)))
                return S_ERROR;

        self->_chunk.pos = chunk->data;
        self->_chunk.end = self->_chunk.pos + data_size;
        self->_chunk.size = data_size;
        list_push_back(&self->_chunks, &chunk->node);
        return S_NO_ERROR;
}

extern void objpool_init(objpool* self, ssize obsize)
{
        objpool_init_ex(self, obsize, STDALLOC);
}

extern void objpool_init_ex(objpool* self, ssize obsize, allocator* alloc)
{
        obstack_init_ex(&self->_base, alloc);
        self->_obsize = obsize;
        self->_top = NULL;
}

extern void objpool_dispose(objpool* self)
{
        obstack_dispose(&self->_base);
}

extern void mempool_init(
        mempool* self, mempool_bad_alloc_handler handler, jmp_buf on_bad_alloc)
{
        mempool_init_ex(self, handler, on_bad_alloc, STDALLOC);
}

extern void mempool_init_ex(
        mempool* self, mempool_bad_alloc_handler handler, jmp_buf on_bad_alloc, allocator* alloc)
{
        S_ASSERT(on_bad_alloc);
        self->_alloc = alloc;
        self->_handler = handler;
        self->_on_bad_alloc = on_bad_alloc;
        allocator_init(&self->_base, &mempool_allocate, &mempool_deallocate);
        list_init(&self->_used);
}

extern void mempool_dispose(mempool* self)
{
        while (!list_empty(&self->_used))
                deallocate(self->_alloc, list_pop_front(&self->_used));
}
