#include "scc/scl/alloc.h"
#include "scc/scl/malloc.h"

static void deallocate_plug(allocator* self, void* p)
{
}

extern void bpa_init(bp_allocator* self)
{
        bpa_init_ex(self, get_std_alloc());
}

extern void bpa_init_ex(bp_allocator* self, allocator* alloc)
{
        allocator_init_ex(bpa_base(self), &bp_allocate, &bp_allocate_ex, &deallocate_plug);
        list_init(&self->_chunks);
        self->_alloc = alloc;
        self->_end = NULL;
        self->_pos = NULL;
}

extern void bpa_dispose(bp_allocator* self)
{
        //todo
}

extern _bp_chunk* _bpa_new_chunk(bp_allocator* self, ssize bytes)
{
        _bp_chunk* c = allocate(self->_alloc, sizeof(_bp_chunk) + bytes);
        if (!c)
                return NULL;

        list_node_init(&c->_node);
        c->_size = bytes;
        return c;
}

extern void _bpa_use_chunk(bp_allocator* self, _bp_chunk* c)
{
        list_push_back(&self->_chunks, &c->_node);
        self->_pos = c->_bytes;
        self->_end = self->_pos + c->_size;
}

extern serrcode _bpa_grow(bp_allocator* self, ssize cst)
{
        ssize chunk_size = cst;
        if (!list_empty(&self->_chunks))
                chunk_size += ((_bp_chunk*)list_last(&self->_chunks))->_size;

        _bp_chunk* c = _bpa_new_chunk(self, chunk_size);
        if (!c)
                return S_ERROR;

        _bpa_use_chunk(self, c);
        return S_NO_ERROR;
}

extern void obj_allocator_init(obj_allocator* self, ssize obsize)
{
        obj_allocator_init_ex(self, obsize, get_std_alloc());
}

static void* _obj_allocate(obj_allocator* self, ssize bytes)
{
        return obj_allocate(self);
}

static void* _obj_allocate_ex(obj_allocator* self, ssize bytes, ssize align)
{
        return obj_allocate(self);
}

static void _obj_deallocate(obj_allocator* self, void* obj)
{
        obj_deallocate(self, obj);
}

extern void obj_allocator_init_ex(obj_allocator* self, ssize obsize, allocator* alloc)
{
        bpa_init_ex(obj_allocator_base(self), alloc);
        allocator_init_ex(bpa_base(obj_allocator_base(self))
                , &_obj_allocate, &_obj_allocate_ex, &_obj_deallocate);
        self->_top = NULL;
        self->_obsize = obsize;
        self->_objects = 0;
}

extern serrcode _obj_allocator_grow(obj_allocator* self)
{
        ssize real_size = self->_obsize + sizeof(_obj_header);
        ssize count = 1 + self->_objects;
        _bp_chunk* c = _bpa_new_chunk(obj_allocator_base(self), count * real_size);
        if (!c)
                return S_ERROR;

        char* val = c->_bytes;
        ssize i = 0;
        for (; i < count - 1; i++)
        {
                _obj_header* h = (_obj_header*)(val + i * real_size);
                h->_next = (char*)h + real_size;
        }
        _obj_header* last = (_obj_header*)(val + i * real_size);
        last->_next = NULL;

        _bpa_use_chunk(obj_allocator_base(self), c);
        return S_NO_ERROR;
}

static void* std_allocate(void* p, ssize bytes)
{
        return smalloc(bytes);
}

static void* std_allocate_ex(void* p, ssize bytes, ssize align)
{
        return smalloc(bytes);
}

static void std_deallocate(void* p, void* block)
{
        sfree(block);
}

static allocator std_alloc =
{
        ._allocate = (alloc_fn)&std_allocate,
        ._allocate_ex = (alloc_ex_fn)&std_allocate_ex,
        ._deallocate = (dealloc_fn)&std_deallocate
};

extern allocator* get_std_alloc()
{
        return &std_alloc;
}