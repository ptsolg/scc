#include "scc/scl/membuf.h"
#include "scc/scl/sstring.h"
#include <memory.h>

extern void membuf_init_ex(membuf* self, allocator* alloc)
{
        self->_begin = NULL;
        self->_end = NULL;
        self->_alloc = alloc;
}

extern void membuf_init(membuf* self)
{
        membuf_init_ex(self, get_std_alloc());
}

extern void membuf_dispose(membuf* self)
{
        deallocate(self->_alloc, self->_begin);
        self->_begin = NULL;
        self->_end = NULL;
}

extern serrcode membuf_resize(membuf* self, ssize size)
{
        return membuf_resize_ex(self, 0, size);
}

extern serrcode membuf_grow(membuf* self)
{
        return membuf_resize_ex(self, 2, 1);
}

extern serrcode membuf_resize_ex(membuf* self, ssize mul, ssize cst)
{
        ssize old_size = membuf_size(self);
        ssize new_size = cst + mul * old_size;

        uchar* new_buf = allocate(self->_alloc, new_size);
        if (!new_buf)
                return S_ERROR;

        ssize cpy_size = SMIN(old_size, new_size);
        memcpy(new_buf, self->_begin, cpy_size);
        deallocate(self->_alloc, self->_begin);

        self->_begin = new_buf;
        self->_end = new_buf + new_size;

        return S_NO_ERROR;
}

extern void membuf_move(membuf* to, membuf* from)
{
        *to = *from;
}
