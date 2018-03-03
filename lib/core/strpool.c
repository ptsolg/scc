#include "scc/core/strpool.h"
#include <stdarg.h>

typedef struct
{
        size_t size;
        uint8_t data[0];
} strentry_impl;

extern void strpool_init(strpool* self)
{
        strpool_init_ex(self, STDALLOC);
}

extern void strpool_init_ex(strpool* self, allocator* alloc)
{
        strmap_init_alloc(&self->_map, alloc);
        obstack_init_ex(&self->_string_alloc, alloc);
}

extern void strpool_dispose(strpool* self)
{
        strmap_dispose(&self->_map);
        obstack_dispose(&self->_string_alloc);
}

extern bool strpool_get(const strpool* self, strref ref, strentry* result)
{
        result->data = NULL;
        result->size = 0;

        strmap_iter it;
        if (!strmap_find(&self->_map, ref, &it))
                return false;

        strentry_impl* entry = *strmap_iter_value(&it);
        result->data = entry->data;
        result->size = entry->size;
        return true;
}

extern bool strpooled(const strpool* self, strref ref)
{
        strmap_iter placeholder;
        return strmap_find(&self->_map, ref, &placeholder);
}

extern strref strpool_insert(strpool* self, const void* data, size_t size)
{
        strref ref = STRREFL(data, size);
        for (strref i = 1; ; i++)
        {
                strentry pooled;
                if (strpool_get(self, ref, &pooled)
                        && pooled.size == size 
                        && memcmp(pooled.data, data, size) == 0)
                {
                        return ref;
                }
                else if (ref != STRMAP_DELETED && ref != STRMAP_EMPTY)
                        break;

                ref += i;
        }

        strentry_impl* copy = obstack_allocate(&self->_string_alloc, sizeof(strentry_impl) + size);
        if (!copy)
                return STRREF_INVALID;

        copy->size = size;
        memcpy(copy->data, data, size);
        if (S_FAILED(strmap_insert(&self->_map, ref, copy)))
                return STRREF_INVALID;

        return ref;
}
