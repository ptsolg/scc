#include "scc/scl/dseq.h"
#include "scc/scl/misc.h"

extern void dseq_init(dseq* self, ssize obsize)
{
        dseq_init_ex(self, obsize, STDALLOC);
}

extern void dseq_init_ex(dseq* self, ssize obsize, allocator* alloc)
{
        S_ASSERT(obsize);

        membuf_init_ex(dseq_base(self), alloc);
        self->_size = 0;
        self->_obsize = obsize;
}
extern void dseq_dispose(dseq* self)
{
        membuf_dispose(dseq_base(self));
}

extern void dseq_move(dseq* to, dseq* from)
{
        S_ASSERT(dseq_obsize(to) == dseq_obsize(from));
        membuf_move(dseq_base(to), dseq_base(from));
        to->_size = from->_size;
}

extern serrcode dseq_reserve(dseq* self, ssize count)
{
        if (count < dseq_available(self))
                return S_NO_ERROR;

        return membuf_resize(dseq_base(self),
                (dseq_size(self) + count) * dseq_obsize(self));
}

extern serrcode dseq_resize(dseq* self, ssize new_size)
{
        if (new_size >= dseq_total(self))
                if (S_FAILED(membuf_resize(dseq_base(self), dseq_obsize(self) * new_size)))
                        return S_ERROR;

        self->_size = new_size;
        return S_NO_ERROR;
}