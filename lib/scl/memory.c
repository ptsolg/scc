#include "scc/scl/memory.h"
#include "scc/scl/malloc.h"
#include <setjmp.h>

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

extern void object_allocator_init(object_allocator* self, ssize obsize)
{
        object_allocator_init_ex(self, obsize, STDALLOC);
}

extern void object_allocator_init_ex(object_allocator* self, ssize obsize, allocator* alloc)
{
        obstack_init_ex(&self->_base, alloc);
        self->_obsize = obsize;
        self->_top = NULL;
}

extern void object_allocator_dispose(object_allocator* self)
{
        obstack_dispose(&self->_base);
}

extern void base_allocator_init(
        base_allocator* self, bad_alloc_handler handler, void* on_bad_alloc)
{
        base_allocator_init_ex(self, handler, on_bad_alloc, STDALLOC);
}

extern void base_allocator_init_ex(
        base_allocator* self, bad_alloc_handler handler, void* on_bad_alloc, allocator* alloc)
{
        S_ASSERT(on_bad_alloc);
        self->_alloc = alloc;
        self->_handler = handler;
        self->_on_bad_alloc = on_bad_alloc;
        allocator_init(&self->_base, &base_allocate, &base_deallocate);
        list_init(&self->_used);
}

extern void base_allocator_dispose(base_allocator* self)
{
        while (!list_empty(&self->_used))
                deallocate(self->_alloc, list_pop_front(&self->_used));
}

extern void malloc_allocator_init(malloc_allocator* self)
{
        allocator_init(&self->_base, &mallocate, &mdeallocate);
}

extern void malloc_allocator_dispose(malloc_allocator* self)
{
        ;
}

extern void* mallocate(malloc_allocator* self, ssize bytes)
{
        return smalloc(bytes);
}
extern void mdeallocate(malloc_allocator* self, void* block)
{
        sfree(block);
}

static malloc_allocator _stdalloc;
static bool _stdalloc_initialized = false;

extern allocator* _get_stdalloc()
{
        if (!_stdalloc_initialized)
        {
                malloc_allocator_init(&_stdalloc);
                _stdalloc_initialized = true;
        }

        return malloc_allocator_base(&_stdalloc);
}