#include "scc/core/membuf.h"
#include "scc/core/memory.h"
#include <memory.h>

extern void membuf_init_ex(membuf* self, allocator* alloc)
{
        self->_begin = NULL;
        self->_end = NULL;
        self->_alloc = alloc;
}

extern void membuf_init(membuf* self)
{
        membuf_init_ex(self, STDALLOC);
}

extern void membuf_dispose(membuf* self)
{
        deallocate(self->_alloc, self->_begin);
        self->_begin = NULL;
        self->_end = NULL;
}

extern errcode membuf_resize(membuf* self, size_t size)
{
        return membuf_resize_ex(self, 0, size);
}

extern errcode membuf_grow(membuf* self)
{
        return membuf_resize_ex(self, 2, 1);
}

extern errcode membuf_resize_ex(membuf* self, size_t mul, size_t cst)
{
        size_t old_size = membuf_size(self);
        size_t new_size = cst + mul * old_size;

        uchar* new_buf = allocate(self->_alloc, new_size);
        if (!new_buf)
                return EC_ERROR;

        size_t cpy_size = MIN(old_size, new_size);
        memcpy(new_buf, self->_begin, cpy_size);
        deallocate(self->_alloc, self->_begin);

        self->_begin = new_buf;
        self->_end = new_buf + new_size;

        return EC_NO_ERROR;
}

extern void membuf_move(membuf* to, membuf* from)
{
        *to = *from;
}
