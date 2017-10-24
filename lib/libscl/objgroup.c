#include "objgroup.h"

extern void objgroup_init(objgroup* self)
{
        objgroup_init_ex(self, get_std_alloc());
}

extern void objgroup_init_ex(objgroup* self, allocator* alloc)
{
        membuf_init_ex(objgroup_base(self), alloc);
        self->_last = NULL;
}

extern void objgroup_dispose(objgroup* self)
{
        membuf_dispose(objgroup_base(self));
        self->_last = NULL;
}

extern void objgroup_move(objgroup* to, objgroup* from)
{
        membuf_move(objgroup_base(to), objgroup_base(from));
        to->_last = from->_last;
}

extern serrcode objgroup_reserve(objgroup* self, ssize count)
{
        ssize objects = objgroup_size(self);
        if (count >= objgroup_available(self))
        {
                if (S_FAILED(membuf_resize(objgroup_base(self), (objects + count) * sizeof(void*))))
                        return S_ERROR;

                self->_last = objgroup_begin(self) + objects;
        }
        return S_NO_ERROR;
}

extern serrcode objgroup_resize(objgroup* self, ssize size)
{
        if (size >= objgroup_total(self))
                if (S_FAILED(membuf_resize(objgroup_base(self), sizeof(void*) * size)))
                        return S_ERROR;

        self->_last = objgroup_begin(self) + size;
        return S_NO_ERROR;
}
