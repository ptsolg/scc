#include "scc/scl/alloc.h"
#include "scc/scl/malloc.h"
#include <setjmp.h>

static void* _allocate_aligned(allocator* self, ssize bytes, ssize alignment)
{
        void* block = allocate(self, bytes + alignment);
        return block
                ? align_pointer(block, alignment)
                : NULL;
}

extern void allocator_init_ex(
        allocator* self, void* allocate, void* allocate_aligned, void* deallocate)
{
        self->_allocate = (allocate_fn)allocate;
        self->_allocate_aligned = (allocate_aligned_fn)allocate_aligned;
        self->_deallocate = (deallocate_fn)deallocate;
}

extern void allocator_init(allocator* self, void* allocate, void* deallocate)
{
        allocator_init_ex(self, allocate, &_allocate_aligned, deallocate);
}

extern void bump_ptr_allocator_init(bump_ptr_allocator* self)
{
        bump_ptr_allocator_init_ex(self, STDALLOC);
}

extern void bump_ptr_allocator_init_ex(bump_ptr_allocator* self, allocator* alloc)
{
        self->_alloc = alloc;
        self->_chunk.pos = NULL;
        self->_chunk.end = NULL;
        self->_chunk.size = 0;
        allocator_init_ex(&self->_base,
                &bump_ptr_allocate,
                &bump_ptr_allocate_aligned,
                &bump_ptr_deallocate);
        list_init(&self->_chunks);
}

extern void bump_ptr_allocator_dispose(bump_ptr_allocator* self)
{
        while (!list_empty(&self->_chunks))
                deallocate(self->_alloc, list_pop_front(&self->_chunks));
}

extern bool _bump_ptr_allocator_grow(bump_ptr_allocator* self, ssize cst)
{
        struct
        {
                list_node node;
                suint8 data[0];
        } *chunk;

        ssize data_size = cst + self->_chunk.size;
        if (!(chunk = allocate(self->_alloc, sizeof(*chunk) + data_size)))
                return false;

        self->_chunk.pos = chunk->data;
        self->_chunk.end = self->_chunk.pos + data_size;
        self->_chunk.size = data_size;
        list_push_back(&self->_chunks, &chunk->node);
        return true;
}

extern void object_allocator_init(object_allocator* self, ssize obsize)
{
        object_allocator_init_ex(self, obsize, STDALLOC);
}

extern void object_allocator_init_ex(object_allocator* self, ssize obsize, allocator* alloc)
{
        bump_ptr_allocator_init_ex(&self->_base, alloc);
        self->_obsize = obsize;
        self->_top = NULL;
}

extern void object_allocator_dispose(object_allocator* self)
{
        bump_ptr_allocator_dispose(&self->_base);
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